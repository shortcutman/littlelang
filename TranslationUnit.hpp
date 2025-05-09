
//------------------------------------------------------------------------------
// TranslationUnit.hpp
//------------------------------------------------------------------------------

#pragma once

#include "TranslationUnitTypes.hpp"

#include <string_view>
#include <vector>

namespace ll {

class TranslationUnit {
private:
    std::vector<FunctionDefinitionPtr> functions;

public:
    TranslationUnit();

    static TranslationUnit parse_translation_unit(std::string_view& input);
    static FunctionDefinitionPtr parse_function_definition(std::string_view& input);
};

}