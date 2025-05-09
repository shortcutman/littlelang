
//------------------------------------------------------------------------------
// TranslationUnit.tests.cpp
//------------------------------------------------------------------------------

#include "TranslationUnit.hpp"

#include "Statement.hpp"

#include <gtest/gtest.h>

using namespace ll;

TEST(TranslationUnit, parse_function_definition) {
    std::string_view eg = R"(fn main() {})";

    auto funcDef = TranslationUnit::parse_function_definition(eg);
    ASSERT_TRUE(funcDef);
    ASSERT_TRUE(funcDef->block);

    EXPECT_EQ(funcDef->name, "main");
}

TEST(TranslationUnit, parse_function_definition_whitespace) {
    std::string_view eg = R"( fn test () {})";

    auto funcDef = TranslationUnit::parse_function_definition(eg);
    ASSERT_TRUE(funcDef);
    ASSERT_TRUE(funcDef->block);

    EXPECT_EQ(funcDef->name, "test");
}

TEST(TranslationUnit, parse_function_definition_content) {
    std::string_view eg = R"( fn test () { printf("asdf"); })";

    auto funcDef = TranslationUnit::parse_function_definition(eg);
    ASSERT_TRUE(funcDef);
    ASSERT_TRUE(funcDef->block);
    EXPECT_EQ(funcDef->name, "test");

    EXPECT_TRUE(funcDef->block->vars.empty());
    EXPECT_EQ(funcDef->block->parent, nullptr);

    ASSERT_EQ(funcDef->block->statements.size(), 1);
    auto call = dynamic_cast<FunctionCall*>(funcDef->block->statements.front().get());
    ASSERT_TRUE(call);

    EXPECT_EQ(call->functionName, "printf");
    EXPECT_EQ(call->params.size(), 1);
}
