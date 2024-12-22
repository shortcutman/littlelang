
//------------------------------------------------------------------------------
// Parser.cpp
//------------------------------------------------------------------------------


#include "Parser.hpp"

#include <dlfcn.h>

void ParsedBlock::parse_block(std::string_view input) {
    std::array<char, 3> steps = { '(', '=', ' ' };

    while (!input.empty()) {
        auto step = input.find_first_of("(=;");
        if (input[step] == '(') {
            //function call
            auto step2 = input.find_first_of(";");
            std::string_view statement(input.begin(), input.begin() + step2 + 1);
            auto call = parse_function_call(statement);
            statements.push_back(std::move(call));
            input.remove_prefix(step + 1);
        } else if (input[step] == '=') {
            //assignment
            auto step2 = input.find_first_of(";");
            std::string_view statement(input.begin(), input.begin() + step2 + 1);
            auto assignment = parse_variable_const_assignment(statement);
            statements.push_back(std::move(assignment));
            input.remove_prefix(step2 + 1);
        } else if (input[step] == ';') {
            //variable definition
            std::string_view statement(input.begin(), input.begin() + step + 1);
            auto def = parse_variable_definition(statement);
            vars.push_back(def);
            input.remove_prefix(step + 1);
        } else {
            throw std::runtime_error("unknown section");
        }
    }
}

FunctionCallPtr ParsedBlock::parse_function_call(std::string_view input) {
    auto call = std::make_unique<FunctionCall>();

    auto nameEnd = input.find_first_of("(");
    if (nameEnd == std::string_view::npos) {
        throw std::runtime_error("Not a function call.");
    }
    call->functionName = input.substr(0, nameEnd);
    input.remove_prefix(nameEnd + 1);

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* function = dlsym(dlHandle, call->functionName.c_str());
    if (!function) {
        throw std::runtime_error("Unknown function name.");
    }
    call->functionAddr = function;

    auto tokenEnd = input.find_first_of(",)");
    while (tokenEnd != std::string_view::npos) {
        auto token = input.substr(0, tokenEnd);
        while (std::isspace(token[0])) {
            token.remove_prefix(1);
        }

        if (std::isdigit(token[0])) {
            //integer
            std::string valueChars(token);
            auto param = std::make_unique<Int64Param>();
            param->content = std::atoi(valueChars.c_str());
            call->params.push_back(std::move(param));
        } else if (token[0] == '"') {
            //string start
            token.remove_prefix(1);
            auto stringEnd = token.find_first_of('"');
            auto param = std::make_unique<StringParam>();
            param->content = std::string(token.substr(0, stringEnd));
            call->params.push_back(std::move(param));
        } else {
            throw std::runtime_error("Unexpected character.");
        }

        input.remove_prefix(tokenEnd + 1);
        tokenEnd = input.find_first_of(",)");
    }

    if (input[0] != ';') {
        throw std::runtime_error("Unexpected character.");
    }

    return call;
}

VariableDefinition ParsedBlock::parse_variable_definition(std::string_view input) {
    VariableDefinition def;

    std::string currentsymbol;
    auto it = input.begin();
    for (; it != input.end(); it++) {
        if (*it == ' ') {
            break;
        } else {
            currentsymbol += *it;
        }
    }

    if (currentsymbol != "int32") {
        throw std::runtime_error("unexpected type");
    }
    def.type = VariableDefinition::Int32;
    it++;

    currentsymbol = "";
    for (; it != input.end(); it++) {
        if (*it == ';') {
            break;
        } else {
            currentsymbol += *it;
        }
    }
    def.name = currentsymbol;

    return def;
}

VariableConstAssignmentPtr ParsedBlock::parse_variable_const_assignment(std::string_view input) {
    auto assign = std::make_unique<VariableConstAssignment>();

    std::string currentsymbol;
    auto it = input.begin();
    for (; it != input.end(); it++) {
        if (*it == ' ') {
            break;
        } else {
            currentsymbol += *it;
        }
    }
    assign->to = currentsymbol;

    it++;
    if (*it++ != '=' && *it++ != ' ') {
        throw std::runtime_error("unexpected character");
    }

    currentsymbol = "";
    for (; it != input.end(); it++) {
        if (*it == ';') {
            break;
        } else {
            currentsymbol += *it;
        }
    }
    assign->value = atoi(currentsymbol.c_str());

    return assign;
}