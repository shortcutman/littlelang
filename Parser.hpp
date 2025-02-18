
//------------------------------------------------------------------------------
// Parser.hpp
//------------------------------------------------------------------------------

#pragma once

#include "Statement.hpp"
#include "Variables.hpp"

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

class Parser {
public:
    std::unique_ptr<Block> block;

public:
    Parser();

    void parse_block(std::string_view input);

    VariableDefinition parse_variable_definition(std::string_view input);

    FunctionCallPtr parse_function_call(std::string_view& input);
    VariableAssignmentPtr parse_variable_assignment(std::string_view& input);
    ParamPtr parse_parameter(std::string_view input);
    IfChainStatementPtr parse_if_chain(std::string_view& input);
    LoopStatementPtr parse_loop(std::string_view& input);

    std::expected<IfStatementPtr, std::string> parse_comparator(std::string_view& input);
};
