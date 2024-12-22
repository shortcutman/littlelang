
//------------------------------------------------------------------------------
// Parser.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct VariableDefinition {
    enum Type {
        Int32
    };

    std::string name;
    Type type;
};

struct Statement {
    virtual ~Statement() = default;
};

struct FunctionCall : public Statement {
    virtual ~FunctionCall() = default;

    std::string functionName;
    void* functionAddr;
    void* param;
};
typedef std::unique_ptr<FunctionCall> FunctionCallPtr;

struct VariableConstAssignment : public Statement {
    virtual ~VariableConstAssignment() = default;

    std::string to;
    std::uint32_t value;
};
typedef std::unique_ptr<VariableConstAssignment> VariableConstAssignmentPtr;

class ParsedBlock {
public:
    std::vector<std::string> string_heap;
    std::vector<VariableDefinition> vars;
    std::vector<std::unique_ptr<Statement>> statements;

public:
    void parse_block(std::string_view input);

    VariableDefinition parse_variable_definition(std::string_view input);

    FunctionCallPtr parse_function_call(std::string_view input);
    VariableConstAssignmentPtr parse_variable_const_assignment(std::string_view input);
};
