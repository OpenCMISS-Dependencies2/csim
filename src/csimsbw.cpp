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
    CsimWrapper() : model(NULL) {}
    ~CsimWrapper() {
        if (model) delete model;
    }

    csim::Model* model;
    std::map<std::string, int> inputVariables, outputVariables;
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
    return CSIM_SUCCESS;
}

int csim_reset()
{
    return CSIM_SUCCESS;
}

int csim_setValue(const char* variableId, double value)
{
    return CSIM_SUCCESS;
}

int csim_getVariables(char** *outArray, int *outLength)
{
    return CSIM_SUCCESS;
}

int csim_getValues(double* *outArray, int *outLength)
{
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

}
