
//------------------------------------------------------------------------------
// Parser.tests.cpp
//------------------------------------------------------------------------------

#include "Parser.hpp"

#include <dlfcn.h>
#include <gtest/gtest.h>

TEST(Parser, parse_function_call) {
    std::string eg = R"(puts("test");)";
    Parser p;
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
    Parser p;
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
    Parser p;
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
    Parser p;
    auto definition = p.parse_variable_definition(eg);

    EXPECT_EQ(definition.name, "test");
    EXPECT_EQ(definition.type, VariableDefinition::Int64);
}

TEST(Parser, parse_variable_assignment_to_const) {
    std::string eg = R"(test = 123;)";
    Parser p;
    auto assign = p.parse_variable_assignment(eg);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<Int64Param*>(assign->value.get());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->content, 123);
}

TEST(Parser, parse_variable_assignment_to_variable) {
    std::string eg = R"(test = another;)";
    Parser p;
    auto assign = p.parse_variable_assignment(eg);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<StackVariableParam*>(assign->value.get());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->content, "another");
}

TEST(Parser, parse_variable_assignment_to_int64_const_addition) {
    std::string eg = R"(test = 1 + 2;)";
    Parser p;
    auto assign = p.parse_variable_assignment(eg);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<StatementParam*>(assign->value.get());
    ASSERT_NE(value, nullptr);

    auto statement = dynamic_cast<Statement*>(value->statement.get());
    ASSERT_NE(statement, nullptr);

    auto int64calc = dynamic_cast<Int64Calcuation*>(statement);
    ASSERT_NE(int64calc, nullptr);
    EXPECT_EQ(int64calc->operation, Int64Calcuation::Addition);

    auto lhs = dynamic_cast<Int64Param*>(int64calc->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, 1);

    auto rhs = dynamic_cast<Int64Param*>(int64calc->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 2);
}

TEST(Parser, parse_variable_assignment_to_int64_const_modulo) {
    std::string eg = R"(test = 4 % 3;)";
    Parser p;
    auto assign = p.parse_variable_assignment(eg);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<StatementParam*>(assign->value.get());
    ASSERT_NE(value, nullptr);

    auto statement = dynamic_cast<Statement*>(value->statement.get());
    ASSERT_NE(statement, nullptr);

    auto int64calc = dynamic_cast<Int64Calcuation*>(statement);
    ASSERT_NE(int64calc, nullptr);
    EXPECT_EQ(int64calc->operation, Int64Calcuation::Modulo);

    auto lhs = dynamic_cast<Int64Param*>(int64calc->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, 4);

    auto rhs = dynamic_cast<Int64Param*>(int64calc->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 3);
}

class ParamParseIntTest
    : public testing::TestWithParam<std::tuple<std::string_view, int64_t>> {
public:
    std::unique_ptr<Param> param;
    int64_t result;

    void SetUp() override {
        auto [input, r] = GetParam();
        result = r;
        Parser p;
        param = p.parse_parameter(input);
    }
};

TEST_P(ParamParseIntTest, parse_parameter_integer) {
    auto int64param = dynamic_cast<Int64Param*>(param.get());
    ASSERT_NE(int64param, nullptr);
    EXPECT_EQ(int64param->content, result);
}

INSTANTIATE_TEST_SUITE_P(IntegerParameterParse, ParamParseIntTest, ::testing::Values(
    std::make_tuple(" 1 ", 1),
    std::make_tuple(" 1", 1),
    std::make_tuple("1 ", 1),
    std::make_tuple(" 12345 ", 12345)
));

class ParamParseStringTest
    : public testing::TestWithParam<std::tuple<std::string_view, std::string_view>> {
public:
    std::unique_ptr<Param> param;
    std::string_view result;

    void SetUp() override {
        auto [input, r] = GetParam();
        result = r;
        Parser p;
        param = p.parse_parameter(input);
    }
};

TEST_P(ParamParseStringTest, parse_parameter_string) {
    auto stringParam = dynamic_cast<StringParam*>(param.get());
    ASSERT_NE(stringParam, nullptr);
    EXPECT_EQ(stringParam->content, result);
}

INSTANTIATE_TEST_SUITE_P(StringParameterParse, ParamParseStringTest, ::testing::Values(
    std::make_tuple("\" 1 \"", " 1 "),
    std::make_tuple("  \"abcd\" ", "abcd")
));

class ParamParseVariableTest
    : public testing::TestWithParam<std::tuple<std::string_view, std::string_view>> {
public:
    std::unique_ptr<Param> param;
    std::string_view result;

    void SetUp() override {
        auto [input, r] = GetParam();
        result = r;
        Parser p;
        param = p.parse_parameter(input);
    }
};

TEST_P(ParamParseVariableTest, parse_parameter_variable) {
    auto stackParam = dynamic_cast<StackVariableParam*>(param.get());
    ASSERT_NE(stackParam, nullptr);
    EXPECT_EQ(stackParam->content, result);
}

INSTANTIATE_TEST_SUITE_P(VariableParameterParse, ParamParseVariableTest, ::testing::Values(
    std::make_tuple("test", "test"),
    std::make_tuple(" test ", "test"),
    std::make_tuple(" ___test ", "___test")
));

TEST(Parser, parse_if_statement_const_parameters) {
    std::string_view eg = R"(if ( 1 == 1) {})";
    Parser p;
    auto ifchain = p.parse_if_chain(eg);
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs = dynamic_cast<Int64Param*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, 1);

    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 1);

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_TRUE(ifstatement->block.statements.empty());
    EXPECT_EQ(ifstatement->block.parent, &p.block);
}


TEST(Parser, parse_if_statement_const_and_stack_parameters) {
    std::string_view eg = R"(if ( 1 == another) {})";
    Parser p;
    auto ifchain = p.parse_if_chain(eg);
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs = dynamic_cast<Int64Param*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, 1);

    auto rhs = dynamic_cast<StackVariableParam*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, "another");

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_TRUE(ifstatement->block.statements.empty());
    EXPECT_EQ(ifstatement->block.parent, &p.block);
}

TEST(Parser, parse_if_statement_stack_and_const_parameters) {
    std::string_view eg = R"(if ( another == 1) {})";
    Parser p;
    auto ifchain = p.parse_if_chain(eg);
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "another");

    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 1);

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_TRUE(ifstatement->block.statements.empty());
    EXPECT_EQ(ifstatement->block.parent, &p.block);
}

TEST(Parser, parse_if_statement_stack_and_stack_parameters) {
    std::string_view eg = R"(if ( another == test ) {})";
    Parser p;
    auto ifchain = p.parse_if_chain(eg);
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "another");

    auto rhs = dynamic_cast<StackVariableParam*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, "test");

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_TRUE(ifstatement->block.statements.empty());
    EXPECT_EQ(ifstatement->block.parent, &p.block);
}

TEST(Parser, parse_if_statement_stack_and_const_parameters_with_block_contents) {
    std::string_view eg = R"(if ( another == 1) { another = 2; })";
    Parser p;
    auto ifchain = p.parse_if_chain(eg);
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "another");

    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 1);

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_EQ(ifstatement->block.statements.size(), 1);
    EXPECT_EQ(ifstatement->block.parent, &p.block);

    auto statement = dynamic_cast<VariableAssignment*>(ifstatement->block.statements.front().get());
    ASSERT_NE(statement, nullptr);
}

TEST(Parser, parse_if_else_if_statement_const_parameters) {
    std::string_view eg = R"(if ( 1 == 1) {} else if ( 2 == 2 ) {})";
    Parser p;
    auto ifchain = p.parse_if_chain(eg);
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 2);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs = dynamic_cast<Int64Param*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, 1);

    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 1);

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_TRUE(ifstatement->block.statements.empty());
    EXPECT_EQ(ifstatement->block.parent, &p.block);

    auto ifstatement2 = ifchain->_ifstatements.back().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);

    auto lhs2 = dynamic_cast<Int64Param*>(ifstatement2->lhs.get());
    ASSERT_NE(lhs2, nullptr);
    EXPECT_EQ(lhs2->content, 2);

    auto rhs2 = dynamic_cast<Int64Param*>(ifstatement2->rhs.get());
    ASSERT_NE(rhs2, nullptr);
    EXPECT_EQ(rhs2->content, 2);

    EXPECT_TRUE(ifstatement2->block.vars.empty());
    EXPECT_TRUE(ifstatement2->block.statements.empty());
    EXPECT_EQ(ifstatement2->block.parent, &p.block);
}
