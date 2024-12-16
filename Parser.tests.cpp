
//------------------------------------------------------------------------------
// Parser.tests.cpp
//------------------------------------------------------------------------------

#include "Parser.hpp"

#include <dlfcn.h>
#include <gtest/gtest.h>

TEST(Parser, parse_function_call) {
    std::string eg = R"(puts("test"))";
    Parser p;
    auto call = p.parse_function_call(eg);
    EXPECT_EQ(call.functionName, "puts");

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* putsaddr = dlsym(dlHandle, "puts");

    EXPECT_EQ(call.functionAddr, putsaddr);
    EXPECT_EQ(call.param, p.string_heap.back().c_str());
}
