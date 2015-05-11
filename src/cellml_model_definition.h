#ifndef CELLML_MODEL_DEFINITION_H
#define CELLML_MODEL_DEFINITION_H

#include <string>
#include <map>
#include <vector>

class CellmlApiObjects;

/**
 * An internal class to manage the use of CellML models.
 */
class CellmlModelDefinition
{
public:
    CellmlModelDefinition();
    ~CellmlModelDefinition();
    int loadModel(const std::string& url);
    // TODO:
    //std::string getVariableIdByXpath(const std::string& xpath);
    /**
     * Flag the specified variable as being an input for the purposes of code generation. This implies
     * that the variable will have its value set externally to the CellML model. Variables flagged as inputs will be
     * used via the INPUT array in this model's executable function. Attempting to flag a variable after
     * this CellML model has been instantiated into executable code will raise an error.
     * @param variableId The ID of the variable to set as an input. This string should be in the format
     * 'component_name/variable_name'.
     * @return On success, the index of this variable in the executable function's INPUT array (>= 0). On error, a
     * negative status code will be returned.
     */
    int setVariableAsInput(const std::string& variableId);

    /**
     * Flag the specified variable as being an output for the purposes of code generation. This implies
     * that the variable will have its value used externally to the CellML model and result in the given variable
     * having an entry in the OUTOUT array in this model's executable function. Attempting to flag a variable after
     * this CellML model has been instantiated into executable code will raise an error.
     * @param variableId The ID of the variable to set as an output. This string should be in the format
     * 'component_name/variable_name'.
     * @return On success, the index of this variable in the executable function's OUTPUT array (>= 0). On error, a
     * negative status code will be returned.
     */
    int setVariableAsOutput(const std::string& variableId);

private:
    std::string mUrl;
    /**
     * Will only be true if a model was completely loaded successfully.
     */
    bool mModelLoaded;

    // we don't want to expose users to the gory details of the CellML API
    CellmlApiObjects* mCapi;

    // TODO: a better way to get "initial values" is to evaluate the model's executable function and pull
    // out the values from there (which will handle initial assignments that are not done using the initial_value
    // attribute.
    //std::map<std::pair<int,int>, double> mInitialValues;
    std::map<std::string, int> mVariableTypes;
    std::map<std::string, int> mVariableIndices;

    int mNumberOfOutputVariables;
    int mNumberOfInputVariables;
    int mNumberOfIndependentVariables;
    int mStateCounter;
};

#endif // CELLML_MODEL_DEFINITION_H
