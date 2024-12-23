
//------------------------------------------------------------------------------
// Compilerx64.tests.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "InstrBufferx64.hpp"
#include "Parser.hpp"

#include <expected>
#include <gtest/gtest.h>

TEST(Compilerx64Tests, compile_function_call_with_intparam) {
    FunctionCall call;
    call.functionName = "testFunc";
    call.functionAddr = reinterpret_cast<void*>(0x1122334455667788);

    auto intparam = std::make_unique<Int64Param>();
    intparam->content = 0xaabbccddeeffaabb;
    call.params.push_back(std::move(intparam));

    ParsedBlock block;
    InstrBufferx64 buffer;
    compiler_x64::compile_function_call(block, call, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, imm64
            0x48, 0xbf, 0xbb, 0xaa, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, //mov rdi, imm64
            0xff, 0xd0 //call rax
        }));
}

TEST(Compilerx64Tests, compile_function_call_with_stringparam) {
    FunctionCall call;
    call.functionName = "testFunc";
    call.functionAddr = reinterpret_cast<void*>(0x1122334455667788);

    auto stringparam = std::make_unique<StringParam>();
    stringparam->content = "string";
    uint8_t* pointer = reinterpret_cast<uint8_t*>(const_cast<char*>(stringparam->content.c_str()));
    call.params.push_back(std::move(stringparam));

    ParsedBlock block;
    InstrBufferx64 buffer;
    compiler_x64::compile_function_call(block, call, buffer);

    std::vector<uint8_t> expected;
    //mov rax, imm64
    expected.insert(expected.end(), {0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11});
    //mov rdi, imm64
    expected.insert(expected.end(), {0x48, 0xbf, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11});
    std::memcpy(&expected[12], &pointer, 8);
    //call rax
    expected.insert(expected.end(), {0xff, 0xd0});

    EXPECT_EQ(
        buffer.buffer(),
        expected);
}

TEST(Compilerx64Tests, compile_multi_args) {
    FunctionCall call;
    call.functionName = "testFunc";
    call.functionAddr = reinterpret_cast<void*>(0x1122334455667788);

    auto intparam1 = std::make_unique<Int64Param>();
    intparam1->content = 1234;
    call.params.push_back(std::move(intparam1));

    auto intparam2 = std::make_unique<Int64Param>();
    intparam2->content = 1234;
    call.params.push_back(std::move(intparam2));

    ParsedBlock block;
    InstrBufferx64 buffer;
    compiler_x64::compile_function_call(block, call, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, imm64
            0x48, 0xbf, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
            0x48, 0xbe, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
            0xff, 0xd0 //call rax
        }));
}

TEST(Compilerx64Tests, compile_block_prefix_0byte_stack) {
    ParsedBlock block;
    InstrBufferx64 buffer;
    compiler_x64::compile_block_prefix(block, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf5, //push rbp
            0x48, 0x89, 0xe5, //mov rbp, rsp
        })
    );
}

TEST(Compilerx64Tests, compile_block_prefix_8byte_stack) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    InstrBufferx64 buffer;
    compiler_x64::compile_block_prefix(block, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf5, //push rbp
            0x48, 0x89, 0xe5, //mov rbp, rsp
            0x48, 0x81, 0xec, 0x10, 0x00, 0x00, 0x00 //sub rsp, 0x10
        })
    );
}

TEST(Compilerx64Tests, compile_block_prefix_16byte_stack) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    VariableDefinition def2;
    def2.name = "test";
    def2.type = VariableDefinition::Int64;
    block.vars.push_back(def2);

    InstrBufferx64 buffer;
    compiler_x64::compile_block_prefix(block, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf5, //push rbp
            0x48, 0x89, 0xe5, //mov rbp, rsp
            0x48, 0x81, 0xec, 0x10, 0x00, 0x00, 0x00 //sub rsp, 0x10
        })
    );
}

TEST(Compilerx64Tests, compile_block_suffix_0byte_stack) {
    ParsedBlock block;
    InstrBufferx64 buffer;
    compiler_x64::compile_block_suffix(block, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x5d, //pop rbp
            0xc3 //ret
        })
    );
}

TEST(Compilerx64Tests, compile_block_suffix_8byte_stack) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    InstrBufferx64 buffer;
    compiler_x64::compile_block_suffix(block, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x81, 0xc4, 0x10, 0x00, 0x00, 0x00, //add rbp, 0x10
            0x5d, //pop rbp
            0xc3 //ret
        })
    );
}

TEST(Compilerx64Tests, compile_block_suffix_16byte_stack) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    VariableDefinition def2;
    def2.name = "test";
    def2.type = VariableDefinition::Int64;
    block.vars.push_back(def2);

    InstrBufferx64 buffer;
    compiler_x64::compile_block_suffix(block, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x81, 0xc4, 0x10, 0x00, 0x00, 0x00, //add rbp, 0x10
            0x5d, //pop rbp
            0xc3 //ret
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_const_int64) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test";
    auto value = std::make_unique<Int64Param>();
    value->content = 1234;
    assign->value = std::move(value);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    compiler_x64::compile_assignment(block, *rawAssign, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_no_var) {
    ParsedBlock block;

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test";
    auto value = std::make_unique<Int64Param>();
    value->content = 1234;
    assign->value = std::move(value);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    EXPECT_THROW(compiler_x64::compile_assignment(block, *rawAssign, buffer), std::bad_expected_access<std::string>);
}

TEST(Compilerx64Tests, compile_assignment_const_int64_by_two) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    VariableDefinition def2;
    def2.name = "test2";
    def2.type = VariableDefinition::Int64;
    block.vars.push_back(def2);

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test2";
    auto value = std::make_unique<Int64Param>();
    value->content = 1234;
    assign->value = std::move(value);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    compiler_x64::compile_assignment(block, *rawAssign, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf0 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_stack_var) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    VariableDefinition def2;
    def2.name = "another";
    def2.type = VariableDefinition::Int64;
    block.vars.push_back(def2);

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test";
    auto value = std::make_unique<StackVariableParam>();
    value->content = "another";
    assign->value = std::move(value);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    compiler_x64::compile_assignment(block, *rawAssign, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x8b, 0x45, 0xf0, // mov rax, [rbp - 16]
            0x48, 0x89, 0x45, 0xf8  //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_int_const_operation) {
    ParsedBlock block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test";

    auto statementParam = std::make_unique<StatementParam>();
    auto int64calc = std::make_unique<Int64Calcuation>();
    int64calc->set_op_from_char('+');
    
    auto lhs = std::make_unique<Int64Param>();
    lhs->content = 1;
    int64calc->lhs = std::move(lhs);

    auto rhs = std::make_unique<Int64Param>();
    rhs->content = 2;
    int64calc->rhs = std::move(rhs);

    statementParam->statement = std::move(int64calc);
    assign->value = std::move(statementParam);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    compiler_x64::compile_assignment(block, *rawAssign, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, 0x1
            0x48, 0xb9, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rcx, 0x1
            0x48, 0x03, 0xc1, //add rax, rcx
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 8], rax
        })
    );
}
