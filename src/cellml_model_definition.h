#ifndef CELLML_MODEL_DEFINITION_H
#define CELLML_MODEL_DEFINITION_H

#include <string>
#include <map>
#include <vector>

#include "compiler.h"

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

    /**
     * Get the type of the specified variable.
     *
     * The variable type is a bit field which may include several types depending on the variable. Bitwise and operator
     * should be used with csim::VariableTypes to check for specific types. e.g., (variableType & csim::StateType).
     *
     * @param variableId The ID of the variable in the format 'component_name/variable_name'.
     * @return The bitwise type definition for the specified variable. The type csim::UndefinedType will be returned
     * on error.
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
     * Instantiate this model defintion into executable coode. Will cause code to be generated and compiled into
     * an executable function.
     * @param compiler The compiler to use for instantiating the model
     * @return CSIM_OK on success.
     */
    int instantiate(Compiler& compiler);

    /**
     * The number of state variables in this model. Will only be correct if a model has successfully been loaded.
     * @return The number of state variables in this model.
     */
    inline int numberOfStateVariables() const
    {
        if (mModelLoaded) return mStateCounter;
        else return -1;
    }

    /**
     * The number of variables currently flagged as inputs in this model. Will only be "complete" if a model has
     * successfully been instantiated.
     * @return The number of input variables in this model.
     */
    inline int numberOfInputVariables() const
    {
        return mNumberOfInputVariables;
    }

    /**
     * The number of variables currently flagged as outputs in this model. Will only be "complete" if a model has
     * successfully been instantiated.
     * @return The number of output variables in this model.
     */
    inline int numberOfOutputVariables() const
    {
        return mNumberOfOutputVariables;
    }

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
    std::map<std::string, unsigned char> mVariableTypes;
    std::map<std::string, std::map<unsigned char, int> > mVariableIndices;

    int mNumberOfOutputVariables;
    int mNumberOfInputVariables;
    int mNumberOfIndependentVariables;
    int mStateCounter;
};

#endif // CELLML_MODEL_DEFINITION_H
