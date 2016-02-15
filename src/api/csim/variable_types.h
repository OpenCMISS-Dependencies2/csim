#ifndef CSIM_VARIABLE_TYPES_H
#define CSIM_VARIABLE_TYPES_H

#include "csim/csim_export.h"

//! Everything in CSim is in this namespace.
namespace csim {

/**
 * In order to be flexible in allowing a single variable to be multiple types (state
 * and output; input and output; etc.) we use these bit flags.
 * http://www.cplusplus.com/forum/general/1590/
 */
enum CSIM_EXPORT VariableTypes {
    UndefinedType    = 0x01,
    StateType        = 0x02,
    InputType        = 0x04,
    OutputType       = 0x08,
    IndependentType  = 0x10
    //OpDaylight      = 0x20
};

} // namespace csim

#endif // CSIM_VARIABLE_TYPES_H
