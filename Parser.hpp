
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
    FunctionCall parse_function_call(const std::string& input);
    VariableDefinition parse_variable_definition(const std::string& input);
    VariableConstAssignment parse_variable_const_assignment(const std::string& input);
};
