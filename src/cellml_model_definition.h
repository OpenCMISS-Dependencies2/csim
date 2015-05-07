#ifndef CELLML_MODEL_DEFINITION_H
#define CELLML_MODEL_DEFINITION_H

#include <string>

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
    void* mModel;
    void* mAnnotations;
    void* mCevas;
    void* mCodeInformation;

};

#endif // CELLML_MODEL_DEFINITION_H
