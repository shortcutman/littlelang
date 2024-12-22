
//------------------------------------------------------------------------------
// ParsedBlock.tests.cpp
//------------------------------------------------------------------------------

#include "Parser.hpp"

#include <vector>
#include <string_view>

#include <gtest/gtest.h>

TEST(ParsedBlock, parse_many) {
    // std::string eg = R"(int32 test;test = 123;)";
    std::string_view view(R"(int32 test;test = 123;)");

    ParsedBlock block;
    block.parse_block(view);

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars.front().name, "test");
    EXPECT_EQ(block.vars.front().type, VariableDefinition::Int32);

    EXPECT_EQ(block.statements.size(), 1);

    auto assign = dynamic_cast<VariableConstAssignment*>(block.statements.front().get());
    EXPECT_NE(assign, nullptr);
    EXPECT_EQ(assign->to, "test");
    EXPECT_EQ(assign->value, 123);
}

TEST(ParsedBlock, parse_many2) {
    std::string eg = R"(int32 test;int32 another;test = 123;another = 1111;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int32);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int32);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableConstAssignment*>(block.statements[0].get());
    EXPECT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to, "test");
    EXPECT_EQ(assign1->value, 123);

    auto assign2 = dynamic_cast<VariableConstAssignment*>(block.statements[1].get());
    EXPECT_NE(assign2, nullptr);
    EXPECT_EQ(assign2->to, "another");
    EXPECT_EQ(assign2->value, 1111);
}

TEST(ParsedBlock, parse_many3) {
    std::string eg = R"(int32 test;test = 123;int32 another;another = 1111;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int32);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int32);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableConstAssignment*>(block.statements[0].get());
    EXPECT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to, "test");
    EXPECT_EQ(assign1->value, 123);

    auto assign2 = dynamic_cast<VariableConstAssignment*>(block.statements[1].get());
    EXPECT_NE(assign2, nullptr);
    EXPECT_EQ(assign2->to, "another");
    EXPECT_EQ(assign2->value, 1111);
}

TEST(ParsedBlock, parse_whitespace1) {
    std::string eg =
        R"(int32 test;
        test = 123;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars.front().name, "test");
    EXPECT_EQ(block.vars.front().type, VariableDefinition::Int32);

    EXPECT_EQ(block.statements.size(), 1);

    auto assign = dynamic_cast<VariableConstAssignment*>(block.statements.front().get());
    EXPECT_NE(assign, nullptr);
    EXPECT_EQ(assign->to, "test");
    EXPECT_EQ(assign->value, 123);
}

TEST(ParsedBlock, parse_whitespace2) {
    std::string eg = R"(
    int32 test;
    test = 123;
    int32 another;
    another = 1111;)";
    std::string_view view(eg);

    ParsedBlock block;
    block.parse_block(view);

    EXPECT_EQ(block.vars.size(), 2);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int32);
    EXPECT_EQ(block.vars[1].name, "another");
    EXPECT_EQ(block.vars[1].type, VariableDefinition::Int32);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableConstAssignment*>(block.statements[0].get());
    EXPECT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to, "test");
    EXPECT_EQ(assign1->value, 123);

    auto assign2 = dynamic_cast<VariableConstAssignment*>(block.statements[1].get());
    EXPECT_NE(assign2, nullptr);
    EXPECT_EQ(assign2->to, "another");
    EXPECT_EQ(assign2->value, 1111);
}