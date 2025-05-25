
//------------------------------------------------------------------------------
// Linker.tests.cpp
//------------------------------------------------------------------------------

#include "Linker.hpp"

#include "Parser.hpp"
#include "TranslationUnit.hpp"

#include <memory>
#include <gtest/gtest.h>

TEST(LinkerTests, link_local_jit) {
    ll::TranslationUnit tu;
    tu.functions.push_back(std::make_unique<ll::FunctionDefinition>());
    tu.functions.front()->name = "cool";
    tu.functions.front()->block = std::make_unique<Block>();

    auto obj = ll::Object::compile_translation_unit(tu, Compiler_x64::Mode::JIT);

    EXPECT_EQ(obj.symbols.size(), 1);
    EXPECT_EQ(
        obj.symbols.front(),
        ll::Symbol({
            .name = "cool",
            .offset = 0
        })
    );
    EXPECT_TRUE(obj.buff._externFuncs.empty());
}

TEST(LinkerTests, link_local_object_file) {
    ll::TranslationUnit tu;
    tu.functions.push_back(std::make_unique<ll::FunctionDefinition>());
    tu.functions.front()->name = "cool";
    tu.functions.front()->block = std::make_unique<Block>();

    auto obj = ll::Object::compile_translation_unit(tu, Compiler_x64::Mode::ObjectFile);

    EXPECT_EQ(obj.symbols.size(), 1);
    EXPECT_EQ(
        obj.symbols.front(),
        ll::Symbol({
            .name = "cool",
            .offset = 0
        })
    );
    EXPECT_TRUE(obj.buff._externFuncs.empty());
}

TEST(LinkerTests, link_local_with_external_jit) {
    Parser p;
    p.parse_block(std::string_view{"printf(\"test\");"});

    ll::TranslationUnit tu;
    tu.functions.push_back(std::make_unique<ll::FunctionDefinition>());
    tu.functions.front()->name = "main";
    tu.functions.front()->block = std::move(p.block);

    auto obj = ll::Object::compile_translation_unit(tu, Compiler_x64::Mode::JIT);

    EXPECT_EQ(obj.symbols.size(), 1);
    EXPECT_EQ(
        obj.symbols.front(),
        ll::Symbol({
            .name = "main",
            .offset = 0
        })
    );
    EXPECT_TRUE(obj.buff._externFuncs.empty());
}

TEST(LinkerTests, link_local_with_external_object_file) {
    Parser p;
    p.parse_block(std::string_view{"printf(\"test\");"});

    ll::TranslationUnit tu;
    tu.functions.push_back(std::make_unique<ll::FunctionDefinition>());
    tu.functions.front()->name = "main";
    tu.functions.front()->block = std::move(p.block);

    auto obj = ll::Object::compile_translation_unit(tu, Compiler_x64::Mode::ObjectFile);

    EXPECT_EQ(obj.symbols.size(), 1);
    EXPECT_EQ(
        obj.symbols.front(),
        ll::Symbol({
            .name = "main",
            .offset = 0
        })
    );
    EXPECT_EQ(obj.buff._externFuncs.size(), 1);
    EXPECT_EQ(
        obj.buff._externFuncs.front(),
        InstrBufferx64::ExternFunction({
            .symbol = "printf",
            .location = 16
            })
    );
}

TEST(LinkerTests, link_local_functions_jit) {
    std::string program_text = R"(
    fn cool() {
        printf("cool");
    }

    fn main() {
        printf("main");
        cool();
    }
    )";

    auto sv = std::string_view{program_text};
    auto tu = ll::TranslationUnit::parse_translation_unit(sv);
    auto obj = ll::Object::compile_translation_unit(*tu, Compiler_x64::Mode::JIT);

    EXPECT_EQ(obj.symbols.size(), 2);
    EXPECT_EQ(
        obj.symbols.front(),
        ll::Symbol({
            .name = "cool",
            .offset = 0
        })
    );
    EXPECT_EQ(
        obj.symbols.back(),
        ll::Symbol({
            .name = "main",
            .offset = 29
        })
    );
    EXPECT_TRUE(obj.buff._externFuncs.empty());
}

TEST(LinkerTests, link_local_functions_object_file) {
    std::string program_text = R"(
    fn cool() {
        printf("cool");
    }

    fn main() {
        printf("main");
        cool();
    }
    )";

    auto sv = std::string_view{program_text};
    auto tu = ll::TranslationUnit::parse_translation_unit(sv);
    auto obj = ll::Object::compile_translation_unit(*tu, Compiler_x64::Mode::ObjectFile);

    EXPECT_EQ(obj.symbols.size(), 2);
    EXPECT_EQ(
        obj.symbols.front(),
        ll::Symbol({
            .name = "cool",
            .offset = 0
        })
    );
    EXPECT_EQ(
        obj.symbols.back(),
        ll::Symbol({
            .name = "main",
            .offset = 22
        })
    );
    EXPECT_EQ(obj.buff._externFuncs.size(), 2);
    EXPECT_EQ(
        obj.buff._externFuncs.front(),
        InstrBufferx64::ExternFunction({
            .symbol = "printf",
            .location = 16
            })
    );
    EXPECT_EQ(
        obj.buff._externFuncs.back(),
        InstrBufferx64::ExternFunction({
            .symbol = "printf",
            .location = 38
            })
    );
}
