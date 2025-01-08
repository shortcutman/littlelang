
//------------------------------------------------------------------------------
// Statement.hpp
//------------------------------------------------------------------------------

#pragma once

#include "Variables.hpp"

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

struct Statement;

struct Block {
    std::vector<VariableDefinition> vars;
    std::vector<std::unique_ptr<Statement>> statements;
    Block* parent = nullptr;

    size_t stack_size_aligned() const {
        size_t size = vars.size() * 8;
        size_t remainder = size % 16;
        return (remainder != 0) ? (size + (16 - remainder)) : size;
    }
};

struct Statement {
    virtual ~Statement() = default;
};

struct Param {
    virtual ~Param() = default;
};
typedef std::unique_ptr<Param> ParamPtr;

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
        Unknown,
        Addition,
        Modulo
    } operation = Unknown;

    static constexpr std::string_view operatorSymbols = "+%";

    inline void set_op_from_char(char op) {
        switch (op) {
            case '+':
                operation = Addition;
                return;
            case '%':
                operation = Modulo;
                return;
            default:
                operation = Unknown;
                return;
        }
    }

    std::unique_ptr<Param> lhs;
    std::unique_ptr<Param> rhs;
};

struct IfStatement : public Statement {
    virtual ~IfStatement() = default;

    enum Comparator {
        None,
        Equal,
        NotEqual,
        LessThan,
        LessThanOrEqual,
        GreaterThan,
        GreaterThanOrEqual,
    } comparator = None;
    std::unique_ptr<Param> lhs;
    std::unique_ptr<Param> rhs;
    std::unique_ptr<Block> block;

    static constexpr std::string_view comparatorSymbols = "=!<>";

    inline size_t set_cmp_from_sv(std::string_view op) {
        if (op.starts_with("==")) {
            comparator = Equal;
            return 2;
        } else if (op.starts_with("!=")) {
            comparator = NotEqual;
            return 2;
        } else if (op.starts_with("<")) {
            comparator = LessThan;
            return 1;
        } else if (op.starts_with("<=")) {
            comparator = LessThanOrEqual;
            return 2;
        } else if (op.starts_with(">")) {
            comparator = GreaterThan;
            return 1;
        } else if (op.starts_with(">=")) {
            comparator = GreaterThanOrEqual;
            return 2;
        } else {
            throw std::runtime_error("unknown comparator");
            return 0;
        }
    }
};
typedef std::unique_ptr<IfStatement> IfStatementPtr;

struct IfChainStatement : public Statement {
    virtual ~IfChainStatement() = default;

    std::vector<std::unique_ptr<IfStatement>> _ifstatements;
};
typedef std::unique_ptr<IfChainStatement> IfChainStatementPtr;

struct LoopStatement : public Statement {
    virtual ~LoopStatement() = default;

    std::unique_ptr<IfStatement> _ifStatement;
};
typedef std::unique_ptr<LoopStatement> LoopStatementPtr;