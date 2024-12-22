
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

struct VariableConstAssignment : public Statement {
    virtual ~VariableConstAssignment() = default;

    std::string to;
    std::uint64_t value;
};
typedef std::unique_ptr<VariableConstAssignment> VariableConstAssignmentPtr;
