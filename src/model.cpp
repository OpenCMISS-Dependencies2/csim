/*
Copyright 2015 University of Auckland

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.Some license of other
*/
#include <iostream>

#include "csim/model.h"
#include "csim/error_codes.h"
#include "cellml_model_definition.h"
#include "compiler.h"

namespace csim {

Model::Model() : mModelDefinition(0), mCompiler(0), mInstantiated(false)
{
}

Model::Model(const Model &src)
{
    mModelDefinition = src.mModelDefinition;
    mCompiler = src.mCompiler;
    mInstantiated = src.mInstantiated;
    mNumberOfStates = 0;
}

Model::~Model()
{
    if (mModelDefinition)
    {
        CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
        delete cellml;
    }
    if (mCompiler)
    {
        Compiler* compiler = static_cast<Compiler*>(mCompiler);
        delete compiler;
    }
}

int Model::loadCellmlModel(const std::string &url)
{
    if (mModelDefinition) delete static_cast<CellmlModelDefinition*>(mModelDefinition);
    std::cout << "Loading CellML Model URL: " << url << std::endl;
    CellmlModelDefinition* cellml = new CellmlModelDefinition();
    int success = cellml->loadModel(url);
    if (success != 0)
    {
        std::cerr << "Model::loadCellmlModel: Unable to load the model: " << url << std::endl;
        delete cellml;
        mModelDefinition = NULL;
        return UNABLE_TO_LOAD_MODEL_URL;
    }
    mModelDefinition = static_cast<void*>(cellml);
    mNumberOfStates = cellml->numberOfStateVariables();
    return CSIM_OK;
}

int Model::setVariableAsInput(const std::string& variableId)
{
    if (mInstantiated) return MODEL_ALREADY_INSTANTIATED;
    if (! mModelDefinition) return MISSING_MODEL_DEFINTION;
    // TODO: need to check that we are using a CellML model...
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    int inputIndex = cellml->setVariableAsInput(variableId);
    return inputIndex;
}

int Model::setVariableAsOutput(const std::string& variableId)
{
    if (mInstantiated) return MODEL_ALREADY_INSTANTIATED;
    if (! mModelDefinition) return MISSING_MODEL_DEFINTION;
    // TODO: need to check that we are using a CellML model...
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    int outputIndex = cellml->setVariableAsOutput(variableId);
    return outputIndex;
}

int Model::instantiate(bool verbose, bool debug)
{
    if (! mModelDefinition) return MISSING_MODEL_DEFINTION;
    // TODO: should first check if using a CellML model...
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    // FIXME: should expose compiler interface to users?
    Compiler* compiler;
    if (!mCompiler)
    {
        compiler = new Compiler(verbose, debug);
        mCompiler = static_cast<void*>(compiler);
    }
    else compiler = static_cast<Compiler*>(mCompiler);
    int code = cellml->instantiate(*compiler);
    if (code == CSIM_OK) mInstantiated = true;
    return code;
}

InitialiseFunction Model::getInitialiseFunction() const
{
    if (! mCompiler) return NULL;
    Compiler* compiler = static_cast<Compiler*>(mCompiler);
    return compiler->getInitialiseFunction();
}

ModelFunction Model::getModelFunction() const
{
    if (! mCompiler) return NULL;
    Compiler* compiler = static_cast<Compiler*>(mCompiler);
    return compiler->getModelFunction();
}

} // namespace csim
