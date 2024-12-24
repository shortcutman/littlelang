
//------------------------------------------------------------------------------
// Parser.hpp
//------------------------------------------------------------------------------

#pragma once

#include "Statement.hpp"
#include "Variables.hpp"

#include <cstdint>
#include <string>
#include <vector>

class Parser {
public:
    Block block;

public:
    void parse_block(std::string_view input);

    VariableDefinition parse_variable_definition(std::string_view input);

    FunctionCallPtr parse_function_call(std::string_view input);
    VariableAssignmentPtr parse_variable_assignment(std::string_view input);
    ParamPtr parse_parameter(std::string_view input);
    IfChainStatementPtr parse_if_chain(std::string_view& input);
};
