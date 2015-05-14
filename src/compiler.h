#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "csim/executable_functions.h"

class Compiler
{
public:
    Compiler(bool verbose, bool debug);
    ~Compiler();

    int compileCodeString(const std::string& code);
    csim::modelFunction getModelFunction();
    csim::initialiseFunction getInitialiseFunction();

private:
    bool mVerbose;
    bool mDebug;
};

#endif // COMPILER_H
