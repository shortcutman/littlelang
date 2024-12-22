
//------------------------------------------------------------------------------
// ParsedBlock.tests.cpp
//------------------------------------------------------------------------------

#include "Parser.hpp"

#include <vector>
#include <string_view>

#include <gtest/gtest.h>

TEST(ParsedBlock, parse_many) {
    std::string_view view(R"(int64 test;test = 123;)");

    ParsedBlock block;
    block.parse_block(view);

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

TEST(ParsedBlock, parse_many2) {
    std::string eg = R"(int64 test;int64 another;test = 123;another = 1111;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

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

TEST(ParsedBlock, parse_many3) {
    std::string eg = R"(int64 test;test = 123;int64 another;another = 1111;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

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

TEST(ParsedBlock, parse_many4) {
    std::string eg = R"(int64 test;puts("test");int64 another;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

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

TEST(ParsedBlock, parse_whitespace1) {
    std::string eg =
        R"(int64 test;
        test = 123;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

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

TEST(ParsedBlock, parse_whitespace2) {
    std::string eg = R"(
    int64 test;
    test = 123;
    int64 another;
    another = 1111;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

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