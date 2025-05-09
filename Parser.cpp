
//------------------------------------------------------------------------------
// Parser.cpp
//------------------------------------------------------------------------------


#include "Parser.hpp"

#include "ParserUtils.hpp"

#include <algorithm>
#include <array>

Parser::Parser()
: block(std::make_unique<Block>())
{
}

void Parser::parse_block(std::string_view input) {
    std::array<char, 3> steps = { '(', '=', ' ' };

    while (!input.empty()) {
        auto step = input.find_first_of("(=;");
        if (step == std::string_view::npos) {
            break;
        } else if (input[step] == '(') {
            auto token = input.substr(0, step);
            trim_sides(token);
            if (token == "if") {
                //if statement
                auto ifchain = parse_if_chain(input);
                block->statements.push_back(std::move(ifchain));
            } else if (token == "while") {
                auto whileStatement = parse_loop(input);
                block->statements.push_back(std::move(whileStatement));
            } else {
                //function call
                auto call = parse_function_call(input);
                block->statements.push_back(std::move(call));
            }
        } else if (input[step] == '=') {
            //assignment
            auto assignment = parse_variable_assignment(input);
            block->statements.push_back(std::move(assignment));
        } else if (input[step] == ';') {
            //variable definition
            auto def = parse_variable_definition(input);
            block->vars.push_back(def);
        } else {
            throw std::runtime_error("unknown section");
        }
    }
}

FunctionCallPtr Parser::parse_function_call(std::string_view& input) {
    auto call = std::make_unique<FunctionCall>();

    auto nameEnd = input.find_first_of("(");
    if (nameEnd == std::string_view::npos) {
        throw std::runtime_error("Not a function call.");
    }

    auto name = input.substr(0, nameEnd);
    trim_sides(name);
    call->functionName = name;
    input.remove_prefix(nameEnd + 1);

    while (!input.empty()) {
        auto tokenEnd = input.find_first_of(",)");
        auto token = input.substr(0, tokenEnd);
        auto param = parse_parameter(token);
        call->params.push_back(std::move(param));
        input.remove_prefix(tokenEnd);
        
        bool finalParam = input[0] == ')';
        input.remove_prefix(1);

        if (finalParam) {
            break;
        }
    }

    trim_left(input);
    if (input[0] != ';') {
        throw std::runtime_error("Unexpected character.");
    }

    input.remove_prefix(1);

    return call;
}

FunctionDefinitionPtr Parser::parse_function_definition(std::string_view& input) {
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
    def->block->parent = this->block.get();
    input.remove_prefix(blockEnd + 1);
    trim_left(input);

    return std::move(def);
}

VariableDefinition Parser::parse_variable_definition(std::string_view& input) {
    VariableDefinition def;

    trim_left(input);
    auto splitter = input.find_first_of(' ');
    auto type = input.substr(0, splitter);

    if (type != "int64") {
        throw std::runtime_error("unexpected type");
    }
    def.type = VariableDefinition::Int64;
    input.remove_prefix(splitter + 1);

    splitter = input.find_first_of(';');
    auto name = input.substr(0, splitter);
    trim_sides(name);

    if (haswhitespace(name)) {
        throw std::runtime_error("Unexpected whitespace.");
    }
    def.name = name;

    input.remove_prefix(splitter + 1);

    return def;
}

VariableAssignmentPtr Parser::parse_variable_assignment(std::string_view& input) {
    auto assign = std::make_unique<VariableAssignment>();

    auto splitter = input.find_first_of('=');
    auto assignTo = input.substr(0, splitter);
    trim_sides(assignTo);
    if (haswhitespace(assignTo)) {
        throw std::runtime_error("Unexpected whitespace.");
    }
    assign->to.content = assignTo;

    input.remove_prefix(splitter + 1);

    auto end = input.find_first_of(';');
    auto value = input.substr(0, end);
    assign->value = parse_parameter(value);
    input.remove_prefix(end + 1);
    return assign;
}

ParamPtr Parser::parse_parameter(std::string_view input) {
    trim_sides(input);
    if (input[0] == '"') {
        if (input.back() != '"') {
            throw std::runtime_error("no end to string found");
        }

        auto param = std::make_unique<StringParam>();
        param->content = std::string(input.substr(1, input.size() - 2));
        return param;
    }
    
    auto opSymbol = input.find_first_of(Int64Calcuation::operatorSymbols);
    if (opSymbol != std::string_view::npos) {
        auto calc = std::make_unique<Int64Calcuation>();
        calc->set_op_from_char(input[opSymbol]);
        auto lhs = input.substr(0, opSymbol);
        calc->lhs = parse_parameter(lhs);
        input.remove_prefix(opSymbol + 1);
        calc->rhs = parse_parameter(input);

        auto statementParam = std::make_unique<StatementParam>();
        statementParam->statement = std::move(calc);
        return statementParam;
    } else if (haswhitespace(input)) {
        throw std::runtime_error("unexpected whitespace");
    } else if (std::isdigit(input[0])) {
        auto int64param = std::make_unique<Int64Param>();
        int64param->content = std::atoi(&input[0]);
        return int64param;
    } else {
        auto param = std::make_unique<StackVariableParam>();
        param->content = input;
        return param;
    }
}

IfChainStatementPtr Parser::parse_if_chain(std::string_view& input) {
    auto ifchain = std::make_unique<IfChainStatement>();

    trim_left(input);
    while (!input.empty()) {
        bool closingElse = false;
        if (input.starts_with("if")) {
            if (!ifchain->_ifstatements.empty()) {
                break;
            }
            input.remove_prefix(2);
        } else if (input.starts_with("else")) {
            if (ifchain->_ifstatements.empty()) {
                throw std::runtime_error("unexpected else / else if statement");
            }

            if (!input.starts_with("else if")) {
                closingElse = true;
                input.remove_prefix(4);
            } else {
                input.remove_prefix(7);
            }
        } else {
            //no longer an if statement
            break;
        }
        
        std::unique_ptr<IfStatement> ifStatement;
        auto ifStatementExpected = parse_comparator(input);
        if (ifStatementExpected.has_value()) {
            if (closingElse) {
                throw std::runtime_error("unexpected comparator");
            } else {
                ifStatement = std::move(ifStatementExpected.value());
            }
        } else {
            ifStatement = std::make_unique<IfStatement>();
            ifStatement->comparator = IfStatement::None;
        }

        trim_left(input);
        auto blockStart = input.find_first_of('{');
        auto blockEnd = find_block_end(input);
        if (blockStart == std::string_view::npos || blockEnd == std::string_view::npos) {
            throw std::runtime_error("couldn't find block delimiters");
        }
        auto block = input.substr(blockStart + 1, blockEnd - blockStart - 1);

        Parser parser;
        parser.parse_block(block);
        ifStatement->block = std::move(parser.block);
        ifStatement->block->parent = this->block.get();
        input.remove_prefix(blockEnd + 1);
        trim_left(input);

        ifchain->_ifstatements.push_back(std::move(ifStatement));

        if (closingElse) {
            break;
        }
    }

    return ifchain;
}

LoopStatementPtr Parser::parse_loop(std::string_view& input) {
    auto loopStatement = std::make_unique<LoopStatement>();

    trim_left(input);
    if (!input.starts_with("while")) {
        throw std::runtime_error("expected while statement");
    }
    input.remove_prefix(5);

    auto ifStatementExpected = parse_comparator(input);
    if (!ifStatementExpected.has_value()) {
        throw ifStatementExpected.error();
    }
    auto ifStatement = std::move(ifStatementExpected.value());

    trim_left(input);
    auto blockStart = input.find_first_of('{');
    auto blockEnd = find_block_end(input);
    if (blockStart == std::string_view::npos || blockEnd == std::string_view::npos) {
        throw std::runtime_error("couldn't find block delimiters");
    }
    auto block = input.substr(blockStart + 1, blockEnd - blockStart - 1);

    Parser parser;
    parser.parse_block(block);
    ifStatement->block = std::move(parser.block);
    ifStatement->block->parent = this->block.get();
    loopStatement->_ifStatement = std::move(ifStatement);
    input.remove_prefix(blockEnd + 1);

    return loopStatement;
}

std::expected<IfStatementPtr, std::string> Parser::parse_comparator(std::string_view& input) {
    auto ifStatement = std::make_unique<IfStatement>();

    trim_left(input);
    if (input[0] != '(') {
        return std::unexpected("Expected opening bracket");
    }
    input.remove_prefix(1);

    auto comparatorEnd = input.find_first_of(')');
    if (comparatorEnd == std::string_view::npos) {
        return std::unexpected("expected closing bracket");
    }

    auto comparison = input.substr(0, comparatorEnd);
    auto comparator = comparison.find_first_of(IfStatement::comparatorSymbols);
    ifStatement->lhs = parse_parameter(comparison.substr(0, comparator));
    comparison.remove_prefix(comparator);
    auto cmpSize = ifStatement->set_cmp_from_sv(comparison.substr(0, 2));
    comparison.remove_prefix(cmpSize);
    ifStatement->rhs = parse_parameter(comparison.substr(0));

    input.remove_prefix(comparatorEnd + 1);

    return ifStatement;
}
