
//------------------------------------------------------------------------------
// TranslationUnitTypes.hpp
//------------------------------------------------------------------------------

#pragma once

#include "Variables.hpp"

#include <vector>
#include <memory>

struct Block;

namespace ll {

struct FunctionDefinition {
    std::string name;
    std::unique_ptr<Block> block;
};
typedef std::unique_ptr<FunctionDefinition> FunctionDefinitionPtr;

}
