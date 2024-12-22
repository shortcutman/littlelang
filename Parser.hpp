
//------------------------------------------------------------------------------
// Parser.hpp
//------------------------------------------------------------------------------

#pragma once

#include "Statement.hpp"
#include "Variables.hpp"

#include <cstdint>
#include <string>
#include <vector>



class ParsedBlock {
public:
    std::vector<VariableDefinition> vars;
    std::vector<std::unique_ptr<Statement>> statements;

public:
    void parse_block(std::string_view input);

    VariableDefinition parse_variable_definition(std::string_view input);

    FunctionCallPtr parse_function_call(std::string_view input);
    VariableAssignmentPtr parse_variable_assignment(std::string_view input);
};
