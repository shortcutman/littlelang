
//------------------------------------------------------------------------------
// TranslationUnit.tests.cpp
//------------------------------------------------------------------------------

#include "TranslationUnit.hpp"

#include "Statement.hpp"

#include <gtest/gtest.h>

using namespace ll;

TEST(TranslationUnit, parse_translation_unit_empty) {
    std::string_view eg = R"()";

    auto tu = TranslationUnit::parse_translation_unit(eg);
    ASSERT_TRUE(tu);
    ASSERT_TRUE(tu->functions.empty());
}

TEST(TranslationUnit, parse_translation_unit_one_function) {
    std::string_view eg = R"(fn main() {})";

    auto tu = TranslationUnit::parse_translation_unit(eg);
    ASSERT_TRUE(tu);
    ASSERT_EQ(tu->functions.size(), 1);

    auto funcDef = tu->functions.front().get();
    ASSERT_TRUE(funcDef);
    EXPECT_EQ(funcDef->name, "main");
}

TEST(TranslationUnit, parse_translation_unit_two_function) {
    std::string_view eg = R"(fn main() {} fn test() {})";

    auto tu = TranslationUnit::parse_translation_unit(eg);
    ASSERT_TRUE(tu);
    ASSERT_EQ(tu->functions.size(), 2);

    auto funcDef = tu->functions.front().get();
    ASSERT_TRUE(funcDef);
    EXPECT_EQ(funcDef->name, "main");

    auto funcDef2 = tu->functions.back().get();
    ASSERT_TRUE(funcDef2);
    EXPECT_EQ(funcDef2->name, "test");
}

TEST(TranslationUnit, parse_translation_unit_parse_error) {
    std::string_view eg = R"(fn main() {} test() {})";
    ASSERT_THROW(TranslationUnit::parse_translation_unit(eg), std::runtime_error);
}

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
