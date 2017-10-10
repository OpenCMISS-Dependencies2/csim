#include <iostream>
#include <map>
#include <cstring>

#include <csimsbw.h>

#include "csim/model.h"
#include "csim/executable_functions.h"
#include "csim/error_codes.h"
#include "xmlutils.h"

#include <cvode/cvode.h>             /* main integrator header file */
#include <cvode/cvode_dense.h>       /* use CVDENSE linear solver */
#include <cvode/cvode_band.h>        /* use CVBAND linear solver */
#include <cvode/cvode_diag.h>        /* use CVDIAG linear solver */
#include <nvector/nvector_serial.h>  /* serial N_Vector types, fct. and macros */
#include <sundials/sundials_types.h> /* definition of realtype */
#include <sundials/sundials_math.h>  /* contains the macros ABS, SUNSQR, and EXP*/

#define CSIM_SUCCESS 0
#define CSIM_FAILED 1

#define ATOL RCONST(1.0e-6)
#define RTOL RCONST(0.0)

/* Function called by CVODE */
static int f(realtype t, N_Vector y, N_Vector ydot, void *user_data);

// assuming we only deal with one model at a time
class CsimWrapper {
public:
    CsimWrapper() : initFunction(NULL), modelFunction(NULL), model(NULL),
        voi(0.0), states(NULL), rates(NULL), inputs(NULL), outputs(NULL),
        maxSteps(1), cvode_mem(nullptr)
    {
        ud = new UserData;
    }
    ~CsimWrapper() {
        if (model) delete model;
        if (states) delete [] states;
        if (rates) delete [] rates;
        if (inputs) delete [] inputs;
        if (outputs) delete [] outputs;
        if (ud) delete ud;
    }

    csim::InitialiseFunction initFunction;
    csim::ModelFunction modelFunction;
    csim::Model* model;
    std::map<std::string, int> inputVariables;
    std::map<std::string, int> outputVariables;
    double voi, *states, *rates, *inputs, *outputs;
    int maxSteps; // currently used to define how many steps to take internally

    // My user data class
    struct UserData
    {
        csim::ModelFunction modelFunction;
        N_Vector states, rates, inputs, outputs;
    };

    void* cvode_mem;
    UserData* ud;

    struct
    {
        double voi;
        std::vector<double> states, rates, inputs, outputs;
    } cache;

    int cacheState()
    {
        cache.voi = voi;
        cache.inputs.clear();
        cache.outputs.clear();
        cache.states.clear();
        cache.rates.clear();
        for (int i=0;i<model->numberOfStateVariables();++i)
        {
            cache.states.push_back(states[i]);
            cache.rates.push_back(rates[i]);
        }
        for (int i=0;i<model->numberOfOutputVariables();++i)
        {
            cache.outputs.push_back(outputs[i]);
        }
        for (int i=0;i<model->numberOfInputVariables();++i)
        {
            cache.inputs.push_back(inputs[i]);
        }
        return CSIM_SUCCESS;
    }

    int popCache()
    {
        voi = cache.voi;
        for (int i=0;i<model->numberOfStateVariables();++i)
        {
            states[i] = cache.states[i];
            rates[i] = cache.rates[i];
        }
        for (int i=0;i<model->numberOfOutputVariables();++i)
        {
            outputs[i] = cache.outputs[i];
        }
        for (int i=0;i<model->numberOfInputVariables();++i)
        {
            inputs[i] = cache.inputs[i];
        }
        return CSIM_SUCCESS;
    }

    void toCvodeData()
    {
        for (int i=0;i<model->numberOfStateVariables();++i)
        {
            NV_Ith_S(ud->states, i) = states[i];
            NV_Ith_S(ud->rates, i) = rates[i];
        }
        for (int i=0;i<model->numberOfOutputVariables();++i)
        {
            NV_Ith_S(ud->outputs, i) = outputs[i];
        }
        for (int i=0;i<model->numberOfInputVariables();++i)
        {
            NV_Ith_S(ud->inputs, i) = inputs[i];
        }
    }

    void fromCvodeData()
    {
        for (int i=0;i<model->numberOfStateVariables();++i)
        {
            states[i] = NV_Ith_S(ud->states, i);
            rates[i] = NV_Ith_S(ud->rates, i);
        }
        for (int i=0;i<model->numberOfOutputVariables();++i)
        {
            outputs[i] = NV_Ith_S(ud->outputs, i);
        }
        for (int i=0;i<model->numberOfInputVariables();++i)
        {
            inputs[i] = NV_Ith_S(ud->inputs, i);
        }
    }

    int integrate(double tOut)
    {
        CVodeSetStopTime(cvode_mem, tOut);
        CVode(cvode_mem, tOut, ud->states, &voi, CV_NORMAL);
        modelFunction(tOut, NV_DATA_S(ud->states), NV_DATA_S(ud->rates),
                      NV_DATA_S(ud->outputs), NV_DATA_S(ud->inputs));
        return CSIM_SUCCESS;
    }
};

static CsimWrapper* _csim = NULL;

int csim_loadCellml(const char* modelString)
{
    if (_csim) delete _csim;
    _csim = new CsimWrapper();
    _csim->model = new csim::Model();
    int code = _csim->model->loadCellmlModelFromString(modelString);
    if (code != csim::CSIM_OK)
    {
        std::cerr << "Error loading the model from a string" << std::endl;
        return CSIM_FAILED;
    }
    // need to flag all the variables before instantiating
    _csim->inputVariables = _csim->model->setAllVariablesAsInput();
    _csim->outputVariables = _csim->model->setAllVariablesAsOutput();
    code = _csim->model->instantiate();
    if (code != csim::CSIM_OK)
    {
        std::cerr << "Error instantiating model" << std::endl;
        return CSIM_FAILED;
    }
    if (_csim->model->numberOfStateVariables() > 0)
    {
        _csim->states = new double[_csim->model->numberOfStateVariables()];
        _csim->rates = new double[_csim->model->numberOfStateVariables()];
    }
    if (_csim->model->numberOfOutputVariables() > 0)
    {
        _csim->inputs = new double[_csim->model->numberOfInputVariables()];
    }
    if (_csim->model->numberOfInputVariables() > 0)
    {
        _csim->outputs = new double[_csim->model->numberOfOutputVariables()];
    }
    _csim->initFunction = _csim->model->getInitialiseFunction();
    _csim->initFunction(_csim->states, _csim->outputs, _csim->inputs);
    _csim->modelFunction = _csim->model->getModelFunction();
    _csim->modelFunction(_csim->voi, _csim->states, _csim->rates, _csim->outputs,
                         _csim->inputs);
    _csim->cacheState();

    _csim->ud->modelFunction = _csim->modelFunction;
    _csim->ud->states = N_VNew_Serial(_csim->model->numberOfStateVariables());
    _csim->ud->rates = N_VNew_Serial(_csim->model->numberOfStateVariables());
    _csim->ud->inputs = N_VNew_Serial(_csim->model->numberOfInputVariables());
    _csim->ud->outputs = N_VNew_Serial(_csim->model->numberOfOutputVariables());
    // now get CVODE set up
    double reltol = RTOL, abstol = ATOL;
    _csim->toCvodeData();
    _csim->cvode_mem = CVodeCreate(CV_ADAMS, CV_FUNCTIONAL);
    CVodeInit(_csim->cvode_mem, f, _csim->voi, _csim->ud->states);
    CVodeSStolerances(_csim->cvode_mem, reltol, abstol);
    CVodeSetUserData(_csim->cvode_mem, (void*)(_csim->ud));
    return CSIM_SUCCESS;
}

int csim_reset()
{
	if (_csim == NULL)
		return CSIM_SUCCESS;
	
    _csim->popCache();

    _csim->toCvodeData();
    CVodeReInit(_csim->cvode_mem, _csim->voi, _csim->ud->states);

    return CSIM_SUCCESS;
}

int csim_setValue(const char* variableId, double value)
{
	if (_csim == NULL)
		return CSIM_SUCCESS;
	
    if (_csim->inputVariables.count(variableId) == 0)
    {
        return CSIM_FAILED;
    }
    int index = _csim->inputVariables[variableId];
    _csim->inputs[index] = value;
    return CSIM_SUCCESS;
}

int csim_getVariables(char** *outArray, int *outLength)
{
	if (_csim == NULL)
		return CSIM_SUCCESS;
	
    // can't use number of outputs directly as variables can be repeated.
    //int length = _csim->model->numberOfOutputVariables();
    int length = _csim->outputVariables.size();
    std::cout << "number output = " << length << std::endl;
    char** variables = (char**)malloc(sizeof(char*)*length);
    char** v = variables;
    for (const auto& ov: _csim->outputVariables)
    {
        //std::cout << "Adding variable " << i << "; with value: " << ov.first << std::endl;
        char *s = strdup(ov.first.c_str());
        *v = s;
        v++;
    }
    *outLength = length;
    *outArray = variables;
    return CSIM_SUCCESS;
}

double* _getValues(int* length)
{
	if (_csim == NULL)
	{
		*length = 0;
		return NULL;
	}
	
    *length = _csim->outputVariables.size();
    double* values = (double*)malloc(sizeof(double)*(*length));
    int i = 0;
    for (const auto& ov: _csim->outputVariables)
    {
        values[i++] = _csim->outputs[ov.second];
    }
    return values;
}

int csim_getValues(double* *outArray, int *outLength)
{
	if (_csim == NULL)
	{
		*outLength = 0;
		*outArray = NULL;
		return CSIM_SUCCESS;
	}
	
    // make sure we are up-to-date
    _csim->modelFunction(_csim->voi, _csim->states, _csim->rates, _csim->outputs,
                         _csim->inputs);
    *outArray = _getValues(outLength);
    return CSIM_SUCCESS;
}

int csim_steadyState()
{
    return CSIM_FAILED;
}

int csim_simulate(
        double initialTime, double startTime, double endTime, int numSteps,
        double** *outMatrix, int* outRows, int *outCols)
{
	if (_csim == NULL)
		return CSIM_SUCCESS;
	
    int length = _csim->outputVariables.size();
    int nData = numSteps + 1;
    double** data = (double**)malloc(sizeof(double*)*nData);
    //for (int i=0; i<length; ++i) data[i] = (double*)malloc(sizeof(double)*nData);
    // set the initial time
    _csim->voi = initialTime;
    if (startTime != initialTime)
    {
        // step to the start time
        _csim->toCvodeData();
        _csim->integrate(startTime);
        _csim->fromCvodeData();
    }
    // grab the values
    data[0] = _getValues(&length);
    //std::cout << std::endl;
    double dt = (endTime - startTime) / ((double)numSteps);
    for (int n=1; n<=numSteps; ++n)
    {
        csim_oneStep(dt);
        data[n] = _getValues(&length);
    }
    *outCols = length;
    *outRows = nData;
    *outMatrix = data;
    return CSIM_SUCCESS;
}

int csim_oneStep(double step)
{
	if (_csim == NULL)
		return CSIM_SUCCESS;
	
    double final = _csim->voi + step;
    _csim->toCvodeData();
    _csim->integrate(final);
    _csim->fromCvodeData();
    return CSIM_SUCCESS;
}

int csim_setTolerances(double aTol, double rTol, int maxSteps)
{
	if (_csim == NULL)
		return CSIM_SUCCESS;
	
    _csim->maxSteps = maxSteps;

    CVodeSStolerances(_csim->cvode_mem, rTol, aTol);
    CVodeSetMaxNumSteps(_csim->cvode_mem, maxSteps);

    return CSIM_SUCCESS;
}

int csim_sayHello(char* *outString, int *outLength)
{
    *outLength = 11;
    *outString = strdup("Hello World");
    return CSIM_SUCCESS;
}

int csim_serialiseCellmlFromUrl(const char* url,
                                char* *outString, int *outLength)
{
    std::string model = csim::Model::serialiseUrlToString(url);
    if (model.empty())
    {
        std::cerr << "Error parsing document at the URL: " << url << std::endl;
        *outLength = 0;
        *outString = NULL;
        return CSIM_FAILED;
    }
    *outLength = model.length();
    *outString = strdup(model.c_str());
    return CSIM_SUCCESS;
}

double csim_getVariableOfIntegration()
{
	if (_csim == NULL)
		return 0;
	
    return _csim->voi;
}

//! Frees a vector previously allocated by this library.
void csim_freeVector(void* vector)
{
    if (vector) free(vector);
}

//! Frees a matrix previously allocated by this library.
void csim_freeMatrix(void** matrix, int numRows)
{
    if (matrix)
    {
        for (int i=0; i<numRows; ++i) free(matrix[i]);
    }
    free(matrix);
}

int f(realtype x, N_Vector y, N_Vector ydot, void *user_data)
{
    CsimWrapper::UserData* ud = (CsimWrapper::UserData*)user_data;
    ud->modelFunction(x, NV_DATA_S(y), NV_DATA_S(ydot), NV_DATA_S(ud->outputs), NV_DATA_S(ud->inputs));
    return 0;
}

