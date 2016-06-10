#include <iostream>
#include <map>
#include <cstring>

#include <csimsbw.h>

#include "csim/model.h"
#include "csim/executable_functions.h"
#include "csim/error_codes.h"
#include "xmlutils.h"

#define CSIM_SUCCESS 0
#define CSIM_FAILED 1

// assuming we only deal with one model at a time
class CsimWrapper {
public:
    CsimWrapper() : initFunction(NULL), modelFunction(NULL), model(NULL),
        voi(0.0), states(NULL), rates(NULL), inputs(NULL), outputs(NULL) {}
    ~CsimWrapper() {
        if (model) delete model;
        if (states) delete [] states;
        if (rates) delete [] rates;
        if (inputs) delete [] inputs;
        if (outputs) delete [] outputs;
    }

    csim::InitialiseFunction initFunction;
    csim::ModelFunction modelFunction;
    csim::Model* model;
    std::map<std::string, int> inputVariables;
    std::map<std::string, int> outputVariables;
    double voi, *states, *rates, *inputs, *outputs;
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
    return CSIM_SUCCESS;
}

int csim_reset()
{
    return CSIM_SUCCESS;
}

int csim_setValue(const char* variableId, double value)
{
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

int csim_getValues(double* *outArray, int *outLength)
{
    int length = _csim->outputVariables.size();
    double* values = (double*)malloc(sizeof(double)*length);
    int i = 0;
    for (const auto& ov: _csim->outputVariables)
    {
        values[i++] = _csim->outputs[ov.second];
    }
    *outLength = length;
    *outArray = values;
    return CSIM_SUCCESS;
}

int csim_steadyState()
{
    return CSIM_SUCCESS;
}

int csim_simulate(
        double initialTime, double startTime, double endTime, int numSteps,
        double** *outMatrix, int* outRows, int *outCols)
{
    return CSIM_SUCCESS;
}

int csim_oneStep(double step)
{
    return CSIM_SUCCESS;
}

int csim_setTolerances(double aTol, double rTol, int maxSteps)
{
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
    XmlDoc xml;
    int code = xml.parseDocument(url);
    if (code != 0)
    {
        std::cerr << "Error parsing document at the URL: " << url << std::endl;
        return CSIM_FAILED;
    }
    xml.setXmlBase(url);
    std::string model = xml.dumpString();
    *outLength = model.length();
    *outString = strdup(model.c_str());
    return CSIM_SUCCESS;
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
