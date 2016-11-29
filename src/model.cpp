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
#include "csim/variable_types.h"
#include "cellml_model_definition.h"
#include "compiler.h"
#include "xmlutils.h"

namespace csim {

Model::Model() : mModelDefinition(0), mCompiler(0), mInstantiated(false), mXmlDoc(0)
{
}

Model::Model(const Model &src)
{
    mModelDefinition = src.mModelDefinition;
    mCompiler = src.mCompiler;
    mInstantiated = src.mInstantiated;
    mNumberOfStates = src.mNumberOfStates;
    mNumberOfInputs = src.mNumberOfInputs;
    mNumberOfOutputs = src.mNumberOfOutputs;
    // FIXME: need to copy the xmldoc?
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
    if (mXmlDoc) delete mXmlDoc;
}

int Model::loadCellmlModel(const std::string &url)
{
    if (mModelDefinition) delete static_cast<CellmlModelDefinition*>(mModelDefinition);
    std::string u = XmlDoc::buildAbsoluteUri(url, "");
    std::cout << "Loading CellML Model URL: " << u << std::endl;
    CellmlModelDefinition* cellml = new CellmlModelDefinition();
    int success = cellml->loadModel(u);
    if (success != 0)
    {
        std::cerr << "Model::loadCellmlModel: Unable to load the model: " << url << std::endl;
        delete cellml;
        mModelDefinition = NULL;
        return UNABLE_TO_LOAD_MODEL_URL;
    }
    mModelDefinition = static_cast<void*>(cellml);
    mNumberOfStates = cellml->numberOfStateVariables();
    if (mXmlDoc) delete mXmlDoc;
    mXmlDoc = new XmlDoc();
    mXmlDoc->parseDocument(u);
    return CSIM_OK;
}

int Model::loadCellmlModelFromString(const std::string &ms)
{
    if (mModelDefinition) delete static_cast<CellmlModelDefinition*>(mModelDefinition);
    std::cout << "Loading CellML Model from given string." << std::endl;
    CellmlModelDefinition* cellml = new CellmlModelDefinition();
    int success = cellml->loadModelFromString(ms);
    if (success != 0)
    {
        std::cerr << "Model::loadCellmlModel: Unable to load the model string." << std::endl;
        delete cellml;
        mModelDefinition = NULL;
        return UNABLE_TO_LOAD_MODEL_STRING;
    }
    mModelDefinition = static_cast<void*>(cellml);
    mNumberOfStates = cellml->numberOfStateVariables();
    if (mXmlDoc) delete mXmlDoc;
    mXmlDoc = new XmlDoc();
    mXmlDoc->parseDocumentString(ms);
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

unsigned char Model::getVariableType(const std::string& variableId)
{
    if (! mModelDefinition) return VariableTypes::UndefinedType;
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    return cellml->getVariableType(variableId);
}

int Model::getVariableIndex(const std::string& variableId, unsigned char variableType)
{
    if (! mModelDefinition) return csim::MISSING_MODEL_DEFINTION;
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    return cellml->getVariableIndex(variableId, variableType);
}

std::map<std::string, int> Model::setAllVariablesAsInput()
{
    std::map<std::string, int> inputVariables;
    if (mInstantiated)
    {
        std::cerr << "Model instantiated, need to set inputs before instantiating"
                  << std::endl;
        return inputVariables;
    }
    if (! mModelDefinition)
    {
        std::cerr << "Missing model definition can't set inputs."
                  << std::endl;
        return inputVariables;
    }
    // TODO: need to check that we are using a CellML model...
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    std::vector<std::string> allVariables = mXmlDoc->getVariableIds();
    for (const auto& id: allVariables)
    {
        // several variables in a model can map to the same input variable.
        int inputIndex = cellml->setVariableAsInput(id);
        if (inputIndex >= 0)
        {
            std::cout << "Input index for " << id << ": " << inputIndex
                      << std::endl;
            inputVariables[id] = inputIndex;
        }
    }
    return inputVariables;
}

std::map<std::string, int> Model::setAllVariablesAsOutput()
{
    std::map<std::string, int> outputVariables;
    if (mInstantiated)
    {
        std::cerr << "Model instantiated, need to set outputs before instantiating"
                  << std::endl;
        return outputVariables;
    }
    if (! mModelDefinition)
    {
        std::cerr << "Missing model definition can't set outputs."
                  << std::endl;
        return outputVariables;
    }
    // TODO: need to check that we are using a CellML model...
    CellmlModelDefinition* cellml = static_cast<CellmlModelDefinition*>(mModelDefinition);
    std::vector<std::string> allVariables = mXmlDoc->getVariableIds();
    for (const auto& id: allVariables)
    {
        // several variables can map to the same output variable
        int outputIndex = cellml->setVariableAsOutput(id);
        if (outputIndex >= 0)
        {
            std::cout << "Output index for " << id << ": " << outputIndex
                      << std::endl;
            outputVariables[id] = outputIndex;
        }
    }
    return outputVariables;
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
    if (code == CSIM_OK)
    {
        mInstantiated = true;
        mNumberOfInputs = cellml->numberOfInputVariables();
        mNumberOfOutputs = cellml->numberOfOutputVariables();
    }
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

std::string Model::mapXpathToVariableId(const std::string &xpath,
                                        const std::map<std::string, std::string>& namespaces)
const
{
    return mXmlDoc->getVariableId(xpath, namespaces);
}

} // namespace csim
