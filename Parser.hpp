
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

class Parser {

public:
    std::vector<std::string> string_heap;

public:
    FunctionCall parse_function_call(const std::string& input);
};
