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

#include <string>

//! Everything in CSim is in this namespace.
namespace csim {

/**
 * The Model class provides the wrapper which makes a CellML model executable.
 *
 * This class provides the executable wrapper for a given CellML model. All functionality provided by the CellML API
 * is hidden from the user by this class.
 */
class Model
{
public:
    /**
     * Default constructor.
     *
     * Construct an empty csim::Model.
     */
    CSIM_EXPORT Model();

    /**
     * Copy constructor
     * @param src The source model to copy.
     */
    CSIM_EXPORT Model(const Model& src);

    /**
     * Destructor.
     */
    CSIM_EXPORT ~Model();

    /**
     * Load the CellML model from the specified URL.
     * @param url The URL from which to load the model.
     * @return zero on success, non-zero if the model is not able to be loaded.
     */
    CSIM_EXPORT int loadCellmlModel(const std::string& url);

    /**
     * Check if this model is able to be compiled into an executable form.
     * @return True if a suitable CellML model has been loaded; false otherwise.
     */
    CSIM_EXPORT bool isModelInstantiated() const;

private:
    /**
     * Internal representation of the CellML model.
     */
    void* mModelDefinition;
};

} // namespace csim

#endif // CSIM_MODEL_H_
