
//------------------------------------------------------------------------------
// Parser.cpp
//------------------------------------------------------------------------------


#include "Parser.hpp"

#include <dlfcn.h>

FunctionCallPtr ParsedBlock::parse_function_call(std::string_view input) {
    auto call = std::unique_ptr<FunctionCall>();
    std::string currentToken;
    auto it = input.begin();
    for (; it != input.end(); it++) {
        char c = *it;
        if (c == '(') {
            break;
        } else {
            currentToken += c;
        }
    }
    call->functionName = currentToken;

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* function = dlsym(dlHandle, currentToken.c_str());
    if (!function) {
        throw std::runtime_error("Unknown function name.");
    }
    call->functionAddr = function;

    it++;
    currentToken = "";
    bool stringOpen = false;
    for (; it != input.end(); it++) {
        char c = *it;
        if (c == '"') {
            if (!stringOpen) {
                stringOpen = true;
                continue;
            } else {
                //close it
                break;
            }
        }

        currentToken += c;
    }

    string_heap.push_back(currentToken);
    call->param = reinterpret_cast<void*>(const_cast<char*>(string_heap.back().c_str()));

    it++;
    if (*it != ')') {
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