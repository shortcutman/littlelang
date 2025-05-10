
//------------------------------------------------------------------------------
// TranslationUnit.cpp
//------------------------------------------------------------------------------

#include "TranslationUnit.hpp"

#include "Parser.hpp"
#include "ParserUtils.hpp"

ll::TranslationUnit::TranslationUnit() {

}

ll::TranslationUnitPtr ll::TranslationUnit::parse_translation_unit(std::string_view& input) {
    auto tu = std::make_unique<TranslationUnit>();

    trim_left(input);
    while (!input.empty()) {
        if (input.substr(0, 3) == "fn ") {
            auto function = TranslationUnit::parse_function_definition(input);
            tu->functions.push_back(std::move(function));
        } else {
            throw std::runtime_error("Unknown input.");
        }
    }

    return tu;
}

ll::FunctionDefinitionPtr ll::TranslationUnit::parse_function_definition(std::string_view& input) {
    auto def = std::make_unique<FunctionDefinition>();

    trim_left(input);
    if (input.substr(0, 3) != "fn ") {
        throw std::runtime_error("expected function definition");
    }

    input.remove_prefix(3);
    auto splitter = input.find_first_of(" (");
    def->name = input.substr(0, splitter);
    
    input.remove_prefix(splitter);
    trim_left(input);
    if (input.substr(0, 2) != "()") {
        throw std::runtime_error("expected empty function parameter definition");
    }
    input.remove_prefix(2);
    trim_left(input);

    auto blockStart = input.find_first_of('{');
    auto blockEnd = find_block_end(input);
    if (blockStart == std::string_view::npos || blockEnd == std::string_view::npos) {
        throw std::runtime_error("couldn't find block delimiters");
    }
    auto block = input.substr(blockStart + 1, blockEnd - blockStart - 1);
    Parser parser;
    parser.parse_block(block);
    def->block = std::move(parser.block);
    input.remove_prefix(blockEnd + 1);
    trim_left(input);

    return std::move(def);
}