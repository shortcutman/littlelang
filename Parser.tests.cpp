
//------------------------------------------------------------------------------
// Parser.tests.cpp
//------------------------------------------------------------------------------

#include "Parser.hpp"

#include <dlfcn.h>
#include <gtest/gtest.h>

TEST(Parser, parse_function_call) {
    std::string eg = R"(puts("test");)";
    ParsedBlock p;
    auto call = p.parse_function_call(eg);
    EXPECT_EQ(call->functionName, "puts");

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* putsaddr = dlsym(dlHandle, "puts");

    EXPECT_EQ(call->functionAddr, putsaddr);
    EXPECT_EQ(call->param, p.string_heap.back().c_str());
    EXPECT_TRUE(strcmp(p.string_heap.back().c_str(), "test") == 0);

    auto stringparam = dynamic_cast<StringParam*>(call->params[0].get());
    EXPECT_NE(stringparam, nullptr);
    EXPECT_EQ(stringparam->content, "test");
}

TEST(Parser, printf_two_args) {
    std::string eg = "printf(\"test %i \n\",123);";
    ParsedBlock p;
    auto call = p.parse_function_call(eg);
    EXPECT_EQ(call->functionName, "printf");

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* printfaddr = dlsym(dlHandle, "printf");

    EXPECT_EQ(call->functionAddr, printfaddr);
    EXPECT_EQ(call->param, p.string_heap.back().c_str());
    EXPECT_EQ(p.string_heap.back(), "test %i \n");

    EXPECT_EQ(call->params.size(), 2);

    auto stringparam = dynamic_cast<StringParam*>(call->params[0].get());
    EXPECT_NE(stringparam, nullptr);
    EXPECT_EQ(stringparam->content, "test %i \n");

    auto intparam = dynamic_cast<Int32Param*>(call->params[1].get());
    EXPECT_NE(intparam, nullptr);
    EXPECT_EQ(intparam->content, 123);
}

TEST(Parser, parse_variable_definition) {
    std::string eg = R"(int32 test;)";
    ParsedBlock p;
    auto definition = p.parse_variable_definition(eg);

    EXPECT_EQ(definition.name, "test");
    EXPECT_EQ(definition.type, VariableDefinition::Int32);
}

TEST(Parser, parse_variable_const_assignment) {
    std::string eg = R"(test = 123;)";
    ParsedBlock p;
    auto assign = p.parse_variable_const_assignment(eg);
    EXPECT_EQ(assign->to, "test");
    EXPECT_EQ(assign->value, 123);
}