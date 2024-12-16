
//------------------------------------------------------------------------------
// Parser.cpp
//------------------------------------------------------------------------------


#include "Parser.hpp"

#include <dlfcn.h>

FunctionCall Parser::parse_function_call(const std::string& input) {
    FunctionCall call;
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
    call.functionName = currentToken;

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* function = dlsym(dlHandle, currentToken.c_str());
    if (!function) {
        throw std::runtime_error("Unknown function name.");
    }
    call.functionAddr = function;

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
        } else {
            currentToken += c;
        }

        currentToken += c;
    }

    string_heap.push_back(currentToken);
    call.param = reinterpret_cast<void*>(const_cast<char*>(string_heap.back().c_str()));

    it++;
    if (*it != ')') {
        throw std::runtime_error("Unexpected character.");
    }

    return call;
}