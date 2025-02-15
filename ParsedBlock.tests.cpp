
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

    auto& block = *parser.block;

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

    auto& block = *parser.block;

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
    auto& block = *parser.block;
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

    auto& block = *parser.block;

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

    auto& block = *parser.block;

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

    auto& block = *parser.block;

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

    auto& block = *parser.block;

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

    EXPECT_TRUE(ifstatement->block->vars.empty());
    EXPECT_EQ(ifstatement->block->statements.size(), 1);
    EXPECT_EQ(ifstatement->block->parent, parser.block.get());

    auto statement = dynamic_cast<FunctionCall*>(ifstatement->block->statements.front().get());
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

    auto& block = *parser.block;

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

    EXPECT_TRUE(ifstatement->block->vars.empty());
    EXPECT_EQ(ifstatement->block->statements.size(), 2);
    EXPECT_EQ(ifstatement->block->parent, parser.block.get());

    auto statement1 = dynamic_cast<VariableAssignment*>(ifstatement->block->statements[0].get());
    ASSERT_NE(statement1, nullptr);

    auto statement2 = dynamic_cast<FunctionCall*>(ifstatement->block->statements[1].get());
    ASSERT_NE(statement2, nullptr);
}

TEST(ParserBlock, parse_multiple_if_blocks) {
    std::string eg = R"(
    int64 test;
    test = 123;
    if (test == 123) {
        printf("test is 123");
    }
    if (test == 444) {
        printf("test is 444");
    }
    )";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = *parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 3);

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

    EXPECT_TRUE(ifstatement->block->vars.empty());
    EXPECT_EQ(ifstatement->block->statements.size(), 1);
    EXPECT_EQ(ifstatement->block->parent, parser.block.get());

    auto statement = dynamic_cast<FunctionCall*>(ifstatement->block->statements.front().get());
    ASSERT_NE(statement, nullptr);

    auto ifchain2 = dynamic_cast<IfChainStatement*>(block.statements[2].get());
    ASSERT_NE(ifchain2, nullptr);
    ASSERT_FALSE(ifchain2->_ifstatements.empty());
    EXPECT_EQ(ifchain2->_ifstatements.size(), 1);

    auto ifstatement2 = ifchain2->_ifstatements.front().get();

    auto lhs2 = dynamic_cast<StackVariableParam*>(ifstatement2->lhs.get());
    ASSERT_NE(lhs2, nullptr);
    EXPECT_EQ(lhs2->content, "test");
    auto rhs2 = dynamic_cast<Int64Param*>(ifstatement2->rhs.get());
    ASSERT_NE(rhs2, nullptr);
    EXPECT_EQ(rhs2->content, 444);

    EXPECT_TRUE(ifstatement2->block->vars.empty());
    EXPECT_EQ(ifstatement2->block->statements.size(), 1);
    EXPECT_EQ(ifstatement2->block->parent, parser.block.get());

    auto statement2 = dynamic_cast<FunctionCall*>(ifstatement2->block->statements.front().get());
    ASSERT_NE(statement2, nullptr);
}

TEST(ParserBlock, parse_nested_if_blocks) {
    std::string eg = R"(
    int64 test;
    test = 123;
    if (test == 123) {
        printf("test is 123");
        if (test == 444) {
            printf("test is 444");
        }
    }
    )";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = *parser.block;

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

    EXPECT_TRUE(ifstatement->block->vars.empty());
    EXPECT_EQ(ifstatement->block->statements.size(), 2);
    EXPECT_EQ(ifstatement->block->parent, parser.block.get());

    auto statement = dynamic_cast<FunctionCall*>(ifstatement->block->statements.front().get());
    ASSERT_NE(statement, nullptr);

    auto ifchain2 = dynamic_cast<IfChainStatement*>(ifstatement->block->statements[1].get());
    ASSERT_NE(ifchain2, nullptr);
    ASSERT_FALSE(ifchain2->_ifstatements.empty());
    EXPECT_EQ(ifchain2->_ifstatements.size(), 1);

    auto ifstatement2 = ifchain2->_ifstatements.front().get();

    auto lhs2 = dynamic_cast<StackVariableParam*>(ifstatement2->lhs.get());
    ASSERT_NE(lhs2, nullptr);
    EXPECT_EQ(lhs2->content, "test");
    auto rhs2 = dynamic_cast<Int64Param*>(ifstatement2->rhs.get());
    ASSERT_NE(rhs2, nullptr);
    EXPECT_EQ(rhs2->content, 444);

    EXPECT_TRUE(ifstatement2->block->vars.empty());
    EXPECT_EQ(ifstatement2->block->statements.size(), 1);
    EXPECT_EQ(ifstatement2->block->parent, ifstatement->block.get());

    auto statement2 = dynamic_cast<FunctionCall*>(ifstatement2->block->statements.front().get());
    ASSERT_NE(statement2, nullptr);
}

TEST(ParserBlock, parse_nested_if_while_blocks) {
    std::string eg = R"(
    int64 test;
    test = 1;
    if (test == 1) {
        while (test < 5) {
            printf("test: %i");
            test = test + 1;
        }
    }
    )";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = *parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 1);

    auto ifchain = dynamic_cast<IfChainStatement*>(block.statements[1].get());
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::Equal);
    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "test");
    auto rhs = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 1);

    EXPECT_TRUE(ifstatement->block->vars.empty());
    EXPECT_EQ(ifstatement->block->statements.size(), 1);
    EXPECT_EQ(ifstatement->block->parent, parser.block.get());

    auto loop = dynamic_cast<LoopStatement*>(ifstatement->block->statements[0].get());
    ASSERT_NE(loop, nullptr);
    ASSERT_NE(loop->_ifStatement, nullptr);

    auto ifstatement2 = loop->_ifStatement.get();
    EXPECT_EQ(ifstatement2->comparator, IfStatement::LessThan);
    auto lhs2 = dynamic_cast<StackVariableParam*>(ifstatement2->lhs.get());
    ASSERT_NE(lhs2, nullptr);
    EXPECT_EQ(lhs2->content, "test");
    auto rhs2 = dynamic_cast<Int64Param*>(ifstatement2->rhs.get());
    ASSERT_NE(rhs2, nullptr);
    EXPECT_EQ(rhs2->content, 5);

    EXPECT_TRUE(ifstatement2->block->vars.empty());
    EXPECT_EQ(ifstatement2->block->statements.size(), 2);
    EXPECT_EQ(ifstatement2->block->parent, ifstatement->block.get());

    auto statement1 = dynamic_cast<FunctionCall*>(ifstatement2->block->statements[0].get());
    ASSERT_NE(statement1, nullptr);

    auto statement2 = dynamic_cast<VariableAssignment*>(ifstatement2->block->statements[1].get());
    ASSERT_NE(statement2, nullptr);
}

TEST(ParserBlock, parse_nested_while_if_blocks) {
    std::string eg = R"(
    int64 test;
    test = 1;
    while (test < 5) {
        if (test == 4) {
            printf("last loop!");
        }
        test = test + 1;
    }
    )";
    std::string_view view(eg);

    Parser parser;
    parser.parse_block(view);

    auto& block = *parser.block;

    EXPECT_EQ(block.vars.size(), 1);
    EXPECT_EQ(block.vars[0].name, "test");
    EXPECT_EQ(block.vars[0].type, VariableDefinition::Int64);

    EXPECT_EQ(block.statements.size(), 2);

    auto assign1 = dynamic_cast<VariableAssignment*>(block.statements[0].get());
    ASSERT_NE(assign1, nullptr);
    EXPECT_EQ(assign1->to.content, "test");
    auto value1 = dynamic_cast<Int64Param*>(assign1->value.get());
    ASSERT_NE(value1, nullptr);
    EXPECT_EQ(value1->content, 1);

    auto loop = dynamic_cast<LoopStatement*>(block.statements[1].get());
    ASSERT_NE(loop, nullptr);
    ASSERT_NE(loop->_ifStatement, nullptr);

    auto ifstatement = loop->_ifStatement.get();
    EXPECT_EQ(ifstatement->comparator, IfStatement::LessThan);
    auto lhs2 = dynamic_cast<StackVariableParam*>(ifstatement->lhs.get());
    ASSERT_NE(lhs2, nullptr);
    EXPECT_EQ(lhs2->content, "test");
    auto rhs2 = dynamic_cast<Int64Param*>(ifstatement->rhs.get());
    ASSERT_NE(rhs2, nullptr);
    EXPECT_EQ(rhs2->content, 5);

    EXPECT_TRUE(ifstatement->block->vars.empty());
    EXPECT_EQ(ifstatement->block->statements.size(), 2);
    EXPECT_EQ(ifstatement->block->parent, &block);

    auto ifchain = dynamic_cast<IfChainStatement*>(ifstatement->block->statements[0].get());
    ASSERT_NE(ifchain, nullptr);
    ASSERT_FALSE(ifchain->_ifstatements.empty());
    EXPECT_EQ(ifchain->_ifstatements.size(), 1);

    auto ifstatement2 = ifchain->_ifstatements.front().get();
    EXPECT_EQ(ifstatement2->comparator, IfStatement::Equal);
    auto lhs = dynamic_cast<StackVariableParam*>(ifstatement2->lhs.get());
    ASSERT_NE(lhs, nullptr);
    EXPECT_EQ(lhs->content, "test");
    auto rhs = dynamic_cast<Int64Param*>(ifstatement2->rhs.get());
    ASSERT_NE(rhs, nullptr);
    EXPECT_EQ(rhs->content, 4);

    EXPECT_TRUE(ifstatement2->block->vars.empty());
    EXPECT_EQ(ifstatement2->block->statements.size(), 1);
    EXPECT_EQ(ifstatement2->block->parent, ifstatement->block.get());

    auto statement2 = dynamic_cast<VariableAssignment*>(ifstatement->block->statements[1].get());
    ASSERT_NE(statement2, nullptr);
}
