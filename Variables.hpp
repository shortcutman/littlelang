//------------------------------------------------------------------------------
// Variables.hpp
//------------------------------------------------------------------------------

#pragma once

#include <string>

struct VariableDefinition {
    enum Type {
        Int64
    };

    std::string name;
    Type type;
};
