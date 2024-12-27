
//------------------------------------------------------------------------------
// ParsedBlock.tests.cpp
//------------------------------------------------------------------------------

#include "Parser.hpp"

#include <vector>
#include <string_view>

#include <gtest/gtest.h>

TEST(ParserBlock, parse_many) {
    std::string_view view(R"(int64 test;test = 123;)");

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars.front().name, "test");
    EXPECT_EQ(block.vars.front().type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 1);

    auto assign = dynamic_cast<VariableAssignment*>(block.statements.front().get());
    ASSERT_NE(assign, nullptr);

    EXPECT_EQ(assign->to.content, "test");

    auto value = dynamic_cast<Int64Param*>(assign->value.get());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->content, 123);
}

TEST(ParserBlock, parse_many2) {
    std::string eg = R"(int64 test;int64 another;test = 123;another = 1111;)";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 123);

    auto assign2 = dynamic_cast<VariableAssignment*>(block.statements[1].get());
    ASSERT_NE(assign2, nullptr);
    EXPECT_EQ(assign2->to.content, "another");
    auto value2 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(value2->content, 123);
}

TEST(ParserBlock, parse_many3) {
    std::string eg = R"(int64 test;test = 123;int64 another;another = 1111;)";
    std::string_view view(eg);

    Parser parser;
    auto& block = parser.block;
    parser.parse_block(view);

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 123);

    auto assign2 = dynamic_cast<VariableAssignment*>(block.statements[1].get());
    ASSERT_NE(assign2, nullptr);
    EXPECT_EQ(assign2->to.content, "another");
    auto value2 = dynamic_cast<Int64Param*>(assign2->value.get());
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(value2->content, 1111);
}

TEST(ParserBlock, parse_many4) {
    std::string eg = R"(int64 test;puts("test");int64 another;)";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 1);

    auto assign1 = dynamic_cast<FunctionCall*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->functionName, "puts");
    EXPECT_EQ(assign1->params.size(), 1);
}

TEST(ParserBlock, parse_whitespace1) {
    std::string eg =
        R"(int64 test;
        test = 123;)";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars.front().name, "test");
    EXPECT_EQ(block.vars.front().type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 1);

    auto assign = dynamic_cast<VariableAssignment*>(block.statements.front().get());
    ASSERT_NE(assign, nullptr);
    EXPECT_EQ(assign->to.content, "test");
    auto value = dynamic_cast<Int64Param*>(assign->value.get());
    ASSERT_NE(value, nullptr);
    EXPECT_EQ(value->content, 123);
}

TEST(ParserBlock, parse_whitespace2) {
    std::string eg = R"(
    int64 test;
    test = 123;
    int64 another;
    another = 1111;)";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 123);

    auto assign2 = dynamic_cast<VariableAssignment*>(block.statements[1].get());
    ASSERT_NE(assign2, nullptr);
    EXPECT_EQ(assign2->to.content, "another");
    auto value2 = dynamic_cast<Int64Param*>(assign2->value.get());
    ASSERT_NE(value2, nullptr);
    EXPECT_EQ(value2->content, 1111);
}

TEST(ParserBlock, parse_if_block) {
    std::string eg = R"(
    int64 test;
    test = 123;
    if (test == 123) {
        printf("test is 123");
    }
    )";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 123);

    auto ifchain = dynamic_cast<IfChainStatement*>(block.statements[1].get());
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();

    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "test");
    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 123);

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_EQ(ifstatement->block.statements.size(), 1);
    EXPECT_EQ(ifstatement->block.parent, &parser.block);

    auto statement = dynamic_cast<FunctionCall*>(ifstatement->block.statements.front().get());
    ASSERT_NE(statement, nullptr);
}

TEST(ParserBlock, parse_if_block_two_statements) {
    std::string eg = R"(
    int64 test;
    test = 123;
    if (test == 123) {
        test = 4444;
        printf("test is %i", test);
    }
    )";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 123);

    auto ifchain = dynamic_cast<IfChainStatement*>(block.statements[1].get());
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();

    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "test");
    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 123);

    EXPECT_TRUE(ifstatement->block.vars.empty());
    EXPECT_EQ(ifstatement->block.statements.size(), 2);
    EXPECT_EQ(ifstatement->block.parent, &parser.block);

    auto statement1 = dynamic_cast<VariableAssignment*>(ifstatement->block.statements[0].get());
    ASSERT_NE(statement1, nullptr);

    auto statement2 = dynamic_cast<FunctionCall*>(ifstatement->block.statements[1].get());
    ASSERT_NE(statement2, nullptr);
}