#ifndef CSIMSBW_H
#define CSIMSBW_H

#include "csim/csim_export.h"

#if __cplusplus
#  define BEGIN_C_DECLS extern "C" { //
#  define END_C_DECLS   } //
#else
#  define BEGIN_C_DECLS
#  define END_C_DECLS
#endif

// C interface
BEGIN_C_DECLS;

// set the given model as the current model and initialise everything
CSIM_EXPORT int csim_loadCellml(const char* modelString);

// reset the model back to initial state (i.e., prior to any simulation or set value)
CSIM_EXPORT int csim_reset();

// will set the value of the given variable, if possible
CSIM_EXPORT int csim_setValue(const char* variableId, double value);

// will return a list of all the output variables for the current model
CSIM_EXPORT int csim_getVariables(char** *outArray, int *outLength);

// get the current value of all the outputs in the current model
CSIM_EXPORT int csim_getValues(double* *outArray, int *outLength);

// Not implemented.
CSIM_EXPORT int csim_steadyState();

// simulate the model over the given interval and return all the  data
// each row of the matrix corresponds to getValues array.
CSIM_EXPORT int csim_simulate(
        double initialTime, double startTime, double endTime, int numSteps,
        double** *outMatrix, int* outRows, int *outCols);

// get the current value of the variable of integration (VOI, usually time)
CSIM_EXPORT double csim_getVariableOfIntegration();

// simulate from the current value of the VoI to Voi + step.
CSIM_EXPORT int csim_oneStep(double step);

// Currently only using the maxSteps
CSIM_EXPORT int csim_setTolerances(double aTol, double rTol, int maxSteps);

CSIM_EXPORT int csim_sayHello(char* *outString, int *outLength);
CSIM_EXPORT int csim_serialiseCellmlFromUrl(const char* url,
                                            char* *outString, int *outLength);

//! Frees a vector previously allocated by this library.
CSIM_EXPORT void csim_freeVector(void* vector);

//! Frees a matrix previously allocated by this library.
CSIM_EXPORT void csim_freeMatrix(void** matrix, int numRows);

END_C_DECLS;

#endif // CSIMSBW_H
