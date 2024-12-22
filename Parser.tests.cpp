
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

    auto stringparam = dynamic_cast<StringParam*>(call->params[0].get());
    ASSERT_NE(stringparam, nullptr);
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

    EXPECT_EQ(call->params.size(), 2);

    auto stringparam = dynamic_cast<StringParam*>(call->params[0].get());
    ASSERT_NE(stringparam, nullptr);
    EXPECT_EQ(stringparam->content, "test %i \n");

    auto intparam = dynamic_cast<Int64Param*>(call->params[1].get());
    ASSERT_NE(intparam, nullptr);
    EXPECT_EQ(intparam->content, 123);
}

TEST(Parser, parse_function_call_stack_argument) {
    std::string eg = "printf(\"test %i \n\",intarg);";
    ParsedBlock p;
    auto call = p.parse_function_call(eg);
    EXPECT_EQ(call->functionName, "printf");

    void* dlHandle = dlopen(0, RTLD_NOW);
    void* printfaddr = dlsym(dlHandle, "printf");
    EXPECT_EQ(call->functionAddr, printfaddr);

    EXPECT_EQ(call->params.size(), 2);

    auto stringparam = dynamic_cast<StringParam*>(call->params[0].get());
    ASSERT_NE(stringparam, nullptr);
    EXPECT_EQ(stringparam->content, "test %i \n");

    auto stackparam = dynamic_cast<StackVariableParam*>(call->params[1].get());
    ASSERT_NE(stackparam, nullptr);
    EXPECT_EQ(stackparam->content, "intarg");
}

TEST(Parser, parse_variable_definition) {
    std::string eg = R"(int64 test;)";
    ParsedBlock p;
    auto definition = p.parse_variable_definition(eg);

    EXPECT_EQ(definition.name, "test");
    EXPECT_EQ(definition.type, VariableDefinition::Int64);
}

TEST(Parser, parse_variable_assignment_to_const) {
    std::string eg = R"(test = 123;)";
    ParsedBlock p;
    auto assign = p.parse_variable_assignment(eg);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<Int64Param*>(assign->value.get());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->content, 123);
}

TEST(Parser, parse_variable_assignment_to_variable) {
    std::string eg = R"(test = another;)";
    ParsedBlock p;
    auto assign = p.parse_variable_assignment(eg);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<StackVariableParam*>(assign->value.get());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->content, "another");
}