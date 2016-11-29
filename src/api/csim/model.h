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

#ifndef CSIM_MODEL_H_
#define CSIM_MODEL_H_

#include "csim/csim_export.h"
#include "csim/executable_functions.h"

#include <string>
#include <map>

class XmlDoc;

//! Everything in CSim is in this namespace.
namespace csim {

/**
 * The Model class provides the wrapper which makes a CellML model executable.
 *
 * This class provides the executable wrapper for a given CellML model. All functionality provided by the CellML API
 * is hidden from the user by this class.
 */
class CSIM_EXPORT Model
{
public:
    /**
     * Default constructor.
     *
     * Construct an empty csim::Model.
     */
     Model();

    /**
     * Copy constructor
     * @param src The source model to copy.
     */
     Model(const Model& src);

    /**
     * Destructor.
     */
     ~Model();

     /**
      * @brief Convenience method to serialise an XML document from the given
      * url to a string.
      * @param url The URL of an XML document to fetch.
      * @param baseUrl The base URL to use when resolving relative URLs. Will
      * default to the current working directory if not provided.
      * @param setXmlBase Determines whether the xml:base attribute is set on the
      * serialised XML document (required for CellML models to resolve imports, invalid
      * in SED-ML).
      * @return The fetched document serialised to a string, with the xml:base
      * property set.
      */
     static std::string serialiseUrlToString(const std::string& url,
                                             const std::string& baseUrl = "",
                                             bool setXmlBase = true);

    /**
     * Load the CellML model from the specified URL.
     * @param url The URL from which to load the model.
     * @return zero on success, non-zero if the model is not able to be loaded.
     */
     int loadCellmlModel(const std::string& url);

     /**
      * Load the CellML model from the given string.
      * @param modelString The string containing the CellML model.
      * @return zero on success, non-zero if the model is not able to be loaded.
      */
      int loadCellmlModelFromString(const std::string& modelString);

     /**
      * Flag the specified variable as being an input for the purposes of code generation. This implies
      * that the variable will have its value set externally to the CellML model. Variables flagged as inputs will be
      * used via the INPUT array in this model's executable function. Attempting to flag a variable after
      * this CellML model has been instantiated into executable code will raise an error.
      * @param variableId The ID of the variable to set as known. This string should be in the format
      * 'component_name/variable_name'.
      * @return On success, the index of this variable in the executable function's INPUT array (>= 0). On error, a
      * negative status code will be returned.
      */
     int setVariableAsInput(const std::string& variableId);

     /**
      * Flag the specified variable as being an output for the purposes of code generation. This implies
      * that the variable will have its value used externally to the CellML model and result in the given variable
      * having an entry in the OUTPUT array in this model's executable function. Attempting to flag a variable after
      * this CellML model has been instantiated into executable code will raise an error.
      * @param variableId The ID of the variable to set as known. This string should be in the format
      * 'component_name/variable_name'.
      * @return On success, the index of this variable in the executable function's OUTPUT array (>= 0). On error, a
      * negative status code will be returned.
      */
     int setVariableAsOutput(const std::string& variableId);

     /**
      * Get the type of the specified variable.
      *
      * The single bitwise type returned should be used with the csim::VariableTypes to determine if the variable is of a
      * specific type. e.g., (variableType & csim::StateType) would be true for state variables. A given variable
      * can have multiple types.
      *
      * @param variableId The ID of the variable in the format 'component_name/variable_name'.
      * @return The bitwise type field of the specified variable. csim::UndefinedType will be returned on error.
      */
     unsigned char getVariableType(const std::string& variableId);

     /**
      * Get the index of the specified variable in its role as the specified type.
      *
      * Each variable may have multiple types. This will return the index of the given variable for its entry in the
      * specified role (0-based index).
      * @param variableId The ID of the variable in the format 'component_name/variable_name'.
      * @param variableType The role of this variable for which you want the index.
      * @return The index of the variable in the specified role. Will be <0 if an error occurs.
      * @see csim::VariableTypes.
      */
     int getVariableIndex(const std::string& variableId, unsigned char variableType);

     /**
      * Set all reachable and suitable variables in the model as inputs.
      *
      * An alternate usage of a CSim model is to flag all suitable variables in the
      * model as input variables. This is useful when you simply want to use the model
      * without knowing a priori the variables of interest. This method will look for
      * all variables in the top-level model (i.e., reachable via SED-ML XPath
      * expressions) and set them to inputs if suitable. The input variable IDs will
      * be returned with their index into the input array.
      *
      * @return A map of the variable IDs for the reachable and suitable variables,
      * and their index in the input array.
      */
     std::map<std::string, int> setAllVariablesAsInput();

     /**
      * Set all reachable and suitable variables in the model as outputs.
      *
      * An alternate usage of a CSim model is to flag all suitable variables in the
      * model as output variables. This is useful when you simply want to use the model
      * without knowing a priori the variables of interest. This method will look for
      * all variables in the top-level model (i.e., reachable via SED-ML XPath
      * expressions) and set them to outputs if suitable. The output variable IDs will
      * be returned.
      *
      * @return A map of the variable IDs for the reachable and suitable variables,
      * and their index in the output array.
      */
     std::map<std::string, int> setAllVariablesAsOutput();

     /**
      * Instantiate the current model into an executable function. This method should only be called once all
      * required inputs and outputs have been set. Once a model is instantiated, no further modifications can be made
      * to the inputs and outputs.
      * @param verbose Tell the compiler to be verbose in its output (defaults to non-verbose output).
      * @param debug Generate a debug version of the executable functions for this model (defaults to optimised).
      * @return csim::CSIM_OK on success, otherwise error code.
      */
     int instantiate(bool verbose = false, bool debug = false);

     /**
      * Return a pointer to the initialisation function for this model.
      * @return A pointer to the initialisation function for this model, NULL on error.
      */
     InitialiseFunction getInitialiseFunction() const;

     /**
      * Get the model function for this model.
      * @return A pointer to the model function, or NULL on error.
      */
     ModelFunction getModelFunction() const;

    /**
     * Check if this model has been instantiated into executable code.
     * @return True if a suitable CellML model has been loaded and instantiated; false otherwise.
     */
     inline bool isInstantiated() const
     {
         return mInstantiated;
     }

     /**
      * Will provide the number of state variables in this model. This is the minimum size of the state and rate
      * arrays. The returned number will only make sense after a model is successfully loaded.
      * @return The number of state variables in this model.
      */
     inline int numberOfStateVariables() const
     {
         return mNumberOfStates;
     }

     /**
      * Will provide the number of variables in this model flagged as input. This is the minimum size of the input
      * array. The returned number will only make sense after a model is successfully loaded.
      * @return The number of input variables in this model.
      */
     inline int numberOfInputVariables() const
     {
         return mNumberOfInputs;
     }

     /**
      * Will provide the number of variables in this model flagged as output. This is the minimum size of the output
      * array. The returned number will only make sense after a model is successfully loaded.
      * @return The number of output variables in this model.
      */
     inline int numberOfOutputVariables() const
     {
         return mNumberOfOutputs;
     }

     std::string mapXpathToVariableId(const std::string& xpath,
                                      const std::map<std::string, std::string>& namespaces) const;

private:
    /**
     * Internal representation of a CellML model.
     */
    void* mModelDefinition;
    void* mCompiler;
    bool mInstantiated;
    int mNumberOfStates, mNumberOfInputs, mNumberOfOutputs;
    XmlDoc* mXmlDoc;
};

} // namespace csim

#endif // CSIM_MODEL_H_
