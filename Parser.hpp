
//------------------------------------------------------------------------------
// Parser.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct FunctionCall {
    std::string functionName;
    void* functionAddr;
    void* param;
};

struct VariableDefinition {
    enum Type {
        Int32
    };

    std::string name;
    Type type;
};

struct VariableConstAssignment {
    std::string to;
    std::uint32_t value;
};

class Parser {

public:
    std::vector<std::string> string_heap;

public:
    FunctionCall parse_function_call(std::string_view input);
    VariableDefinition parse_variable_definition(std::string_view input);
    VariableConstAssignment parse_variable_const_assignment(std::string_view input);
};
