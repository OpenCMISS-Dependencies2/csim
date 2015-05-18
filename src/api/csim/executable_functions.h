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

#ifndef CSIM_EXECUTABLE_FUNCTIONS_H
#define CSIM_EXECUTABLE_FUNCTIONS_H


#include "csim/csim_export.h"

//! Everything in CSim is in this namespace.
namespace csim {

/**
 * This prototype is used for the main model function - given the current state of the model and specified input values,
 * evaluate the model for the given variable of integration (voi).
 *
 * model(voi, states, rates, outputs, inputs)
 */
typedef void (*ModelFunction)(double, double*, double*, double*, double*);

/**
 * This prototype is used for the model initialise function, i.e., set up the state and input arrays with the default
 * values from the model used to generate this function. Also need outputs in case any constants are flagged as outputs
 * but not inputs.
 *
 * initialise(states, outputs, inputs)
 */
typedef void (*InitialiseFunction)(double*, double*, double*);

} // namespace csim

#endif // CSIM_EXECUTABLE_FUNCTIONS_H
