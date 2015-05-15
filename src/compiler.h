#ifndef COMPILER_H
#define COMPILER_H

#include <string>
#include "csim/executable_functions.h"

class LlvmObjects;

class Compiler
{
public:
    Compiler(bool verbose, bool debug);
    ~Compiler();

    int compileCodeString(const std::string& code);
    csim::ModelFunction getModelFunction();
    csim::InitialiseFunction getInitialiseFunction();
    inline bool isVerbose() const
    {
        return mVerbose;
    }
    inline bool isDebug() const
    {
        return mDebug;
    }

private:
    bool mVerbose;
    bool mDebug;
    LlvmObjects* mLLVM;
};

#endif // COMPILER_H
