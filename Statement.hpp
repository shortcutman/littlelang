
//------------------------------------------------------------------------------
// Statement.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Statement {
    virtual ~Statement() = default;
};

struct Param {
    virtual ~Param() = default;
};

struct StringParam : public Param {
    virtual ~StringParam() = default;

    std::string content;
};

struct Int64Param : public Param {
    virtual ~Int64Param() = default;

    std::int64_t content;
};

struct StatementParam : public Param {
    virtual ~StatementParam() = default;

    std::unique_ptr<Statement> statement;
};

struct StackVariableParam : public Param {
    virtual ~StackVariableParam() = default;
    std::string content;
};

struct FunctionCall : public Statement {
    virtual ~FunctionCall() = default;

    std::string functionName;
    void* functionAddr;
    std::vector<std::unique_ptr<Param>> params;
};
typedef std::unique_ptr<FunctionCall> FunctionCallPtr;

struct VariableAssignment : public Statement {
    virtual ~VariableAssignment() = default;

    StackVariableParam to;
    std::unique_ptr<Param> value;
};
typedef std::unique_ptr<VariableAssignment> VariableAssignmentPtr;

struct Int64Calcuation : public Statement {
    virtual ~Int64Calcuation() = default;

    enum Operation {
        Addition
    } operation;

    std::unique_ptr<Param> lhs;
    std::unique_ptr<Param> rhs;
};
