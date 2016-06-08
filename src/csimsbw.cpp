#include <iostream>
#include <vector>
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
};

static CsimWrapper* _csim = NULL;

int csim_loadCellml(const char* modelString)
{
    if (_csim) delete _csim;
    _csim = new CsimWrapper();
    _csim->model = new csim::Model();
    return _csim->model->loadCellmlModelFromString(modelString);
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
