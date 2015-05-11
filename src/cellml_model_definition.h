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

    int mNumberOfWantedVariables;
    int mNumberOfKnownVariables;
    int mNumberOfIndependentVariables;
    int mStateCounter;
};

#endif // CELLML_MODEL_DEFINITION_H
