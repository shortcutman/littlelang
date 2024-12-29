
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

    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_function_call(call);

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

    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_function_call(call);

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

    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_function_call(call);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, imm64
            0x48, 0xbf, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
            0x48, 0xbe, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
            0xff, 0xd0 //call rax
        }));
}

TEST(Compilerx64Tests, compile_function_prefix) {
    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_function_prefix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf5, //push rbp
            0x48, 0x89, 0xe5, //mov rbp, rsp
        })
    );
}

TEST(Compilerx64Tests, compile_block_prefix_0byte_stack) {
    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_block_prefix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({})
    );
}

TEST(Compilerx64Tests, compile_block_prefix_8byte_stack) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_block_prefix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x81, 0xec, 0x10, 0x00, 0x00, 0x00 //sub rsp, 0x10
        })
    );
}

TEST(Compilerx64Tests, compile_block_prefix_16byte_stack) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    VariableDefinition def2;
    def2.name = "test";
    def2.type = VariableDefinition::Int64;
    block.vars.push_back(def2);

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_block_prefix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x81, 0xec, 0x10, 0x00, 0x00, 0x00 //sub rsp, 0x10
        })
    );
}

TEST(Compilerx64Tests, compile_function_suffix) {
    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_function_suffix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x5d, //pop rbp
            0xc3 //ret
        })
    );
}

TEST(Compilerx64Tests, compile_block_suffix_0byte_stack) {
    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_block_suffix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({})
    );
}

TEST(Compilerx64Tests, compile_block_suffix_8byte_stack) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_block_suffix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x81, 0xc4, 0x10, 0x00, 0x00, 0x00, //add rbp, 0x10
        })
    );
}

TEST(Compilerx64Tests, compile_block_suffix_16byte_stack) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    VariableDefinition def2;
    def2.name = "test";
    def2.type = VariableDefinition::Int64;
    block.vars.push_back(def2);

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_block_suffix();

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x81, 0xc4, 0x10, 0x00, 0x00, 0x00, //add rbp, 0x10
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_const_int64) {
    Block block;

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
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_assignment(*rawAssign);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_no_var) {
    Block block;

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test";
    auto value = std::make_unique<Int64Param>();
    value->content = 1234;
    assign->value = std::move(value);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    EXPECT_THROW(compiler.compile_assignment(*rawAssign), std::bad_expected_access<std::string>);
}

TEST(Compilerx64Tests, compile_assignment_const_int64_by_two) {
    Block block;

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
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_assignment(*rawAssign);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf0 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_stack_var) {
    Block block;

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
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_assignment(*rawAssign);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x8b, 0x45, 0xf0, // mov rax, [rbp - 16]
            0x48, 0x89, 0x45, 0xf8  //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_int_const_operation_addition) {
    Block block;

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
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_assignment(*rawAssign);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf1, //push rcx
            0x48, 0xb8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, 0x1
            0x48, 0xb9, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rcx, 0x1
            0x48, 0x03, 0xc1, //add rax, rcx
            0x59, //pop rcx
            0x48, 0x89, 0x45, 0xf8, //mov [rbp - 8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_assignment_operation_int_const_addition_stack_var) {
    Block block;

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

    auto statementParam = std::make_unique<StatementParam>();
    auto int64calc = std::make_unique<Int64Calcuation>();
    int64calc->set_op_from_char('+');
    
    auto lhs = std::make_unique<Int64Param>();
    lhs->content = 1;
    int64calc->lhs = std::move(lhs);

    auto rhs = std::make_unique<StackVariableParam>();
    rhs->content = "another";
    int64calc->rhs = std::move(rhs);

    statementParam->statement = std::move(int64calc);
    assign->value = std::move(statementParam);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_assignment(*rawAssign);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf1, //push rcx
            0x48, 0xb8, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, 0x1
            0x48, 0x8b, 0x4d, 0xf0, //mov rcx, [rbp - 0x10]
            0x48, 0x03, 0xc1, //add rax, rcx
            0x59, //pop rcx
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

class Compilex64ParamStackTest
    : public testing::TestWithParam<std::tuple<InstrBufferx64::Register, std::vector<uint8_t>>>
{
public:
    InstrBufferx64::Register dest;
    std::vector<uint8_t> check;

    void SetUp() override {
        auto [desiredDest, testCheck] = GetParam();
        dest = desiredDest;
        check = testCheck;
    }
};

TEST_P(Compilex64ParamStackTest, compile_parameter_to_register_stack_variable) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    StackVariableParam param;
    param.content = "test";

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_parameter_to_register(&param, dest);

    EXPECT_EQ(buffer.buffer(), check);
}

INSTANTIATE_TEST_SUITE_P(Compilex64ParamStackTest, Compilex64ParamStackTest, ::testing::Values(
    std::make_tuple(InstrBufferx64::Register::RAX,
                    std::vector<uint8_t>({0x48, 0x8b, 0x45, 0xf8})),
    std::make_tuple(InstrBufferx64::Register::RCX,
                    std::vector<uint8_t>({0x48, 0x8b, 0x4d, 0xf8})),
    std::make_tuple(InstrBufferx64::Register::RDI,
                    std::vector<uint8_t>({0x48, 0x8b, 0x7d, 0xf8}))
));

class Compilex64ParamInt64Test
    : public testing::TestWithParam<std::tuple<InstrBufferx64::Register, std::vector<uint8_t>>>
{
public:
    InstrBufferx64::Register dest;
    std::vector<uint8_t> check;

    void SetUp() override {
        auto [desiredDest, testCheck] = GetParam();
        dest = desiredDest;
        check = testCheck;
    }
};

TEST_P(Compilex64ParamInt64Test, compile_parameter_to_register_const_int) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    Int64Param param;
    param.content = 1234;

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_parameter_to_register(&param, dest);

    EXPECT_EQ(buffer.buffer(), check);
}

INSTANTIATE_TEST_SUITE_P(Compilex64ParamInt64Test, Compilex64ParamInt64Test, ::testing::Values(
    std::make_tuple(InstrBufferx64::Register::RAX,
                    std::vector<uint8_t>({0x48, 0xb8, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})),
    std::make_tuple(InstrBufferx64::Register::RCX,
                    std::vector<uint8_t>({0x48, 0xb9, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00})),
    std::make_tuple(InstrBufferx64::Register::RDI,
                    std::vector<uint8_t>({0x48, 0xbf, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}))
));

TEST(Compilerx64Tests, compile_assignment_int_const_operation_modulo) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto assign = std::make_unique<VariableAssignment>();
    VariableAssignment* rawAssign = assign.get();
    assign->to.content = "test";

    auto statementParam = std::make_unique<StatementParam>();
    auto int64calc = std::make_unique<Int64Calcuation>();
    int64calc->set_op_from_char('%');
    
    auto lhs = std::make_unique<Int64Param>();
    lhs->content = 4;
    int64calc->lhs = std::move(lhs);

    auto rhs = std::make_unique<Int64Param>();
    rhs->content = 3;
    int64calc->rhs = std::move(rhs);

    statementParam->statement = std::move(int64calc);
    assign->value = std::move(statementParam);
    block.statements.push_back(std::move(assign));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_assignment(*rawAssign);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xff, 0xf2, //push rdx
            0xff, 0xf1, //push rcx
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, 0x4
            0x48, 0xb9, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rcx, 0x3
            0x48, 0x99, //cqo
            0x48, 0xf7, 0xf9, //idiv rax, rcx
            0x48, 0x89, 0xd0, //mov rax, rdx
            0x59, //pop rcx
            0x5a, //pop rdx
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, get_stack_location_one_level) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    Compiler_x64 compiler(&block, nullptr);

    EXPECT_EQ(block.parent, nullptr);
    EXPECT_EQ(compiler.get_stack_location("test"), -8);
    EXPECT_ANY_THROW(compiler.get_stack_location("test2").value());
}

TEST(Compilerx64Tests, get_stack_location_two_levels) {
    Block block;
    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    Block block2;
    block2.parent = &block;
    VariableDefinition def2;
    def2.name = "another";
    def2.type = VariableDefinition::Int64;
    block2.vars.push_back(def2);

    Compiler_x64 compiler_onelevel(&block, nullptr);
    EXPECT_EQ(block.parent, nullptr);
    EXPECT_EQ(compiler_onelevel.get_stack_location("test").value(), -8);
    EXPECT_ANY_THROW(compiler_onelevel.get_stack_location("another").value());

    Compiler_x64 compiler_twolevel(&block2, nullptr);
    EXPECT_EQ(block2.parent, &block);
    EXPECT_EQ(compiler_twolevel.get_stack_location("test").value(), -8);
    EXPECT_EQ(compiler_twolevel.get_stack_location("another").value(), -24);
}

TEST(Compilerx64Tests, compile_block_with_if_statement) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto ifchain = std::make_unique<IfChainStatement>();
    auto ifchainraw = ifchain.get();
    auto ifstatement = std::make_unique<IfStatement>();

    ifstatement->comparator = IfStatement::Equal;
    auto stackLhs = std::make_unique<StackVariableParam>();
    stackLhs->content = "test";
    ifstatement->lhs = std::move(stackLhs);
    auto int64Rhs = std::make_unique<Int64Param>();
    int64Rhs->content = 1234;
    ifstatement->rhs = std::move(int64Rhs);

    auto assign = std::make_unique<VariableAssignment>();
    assign->to.content = "test";
    
    auto valueParam = std::make_unique<Int64Param>();
    valueParam->content = 4;
    assign->value = std::move(valueParam);
    ifstatement->block = std::make_unique<Block>();
    ifstatement->block->statements.push_back(std::move(assign));
    ifstatement->block->parent = &block;
    ifchain->_ifstatements.push_back(std::move(ifstatement));
    block.statements.push_back(std::move(ifchain));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_if_chain(ifchainraw);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x8b, 0x45, 0xf8, //mov rax, [rbp - 0x8]
            0x48, 0xb9, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, 1234
            0x48, 0x3b, 0xc1, //cmp rax, rcx
            0x0f, 0x85, 0x0e, 0x00, 0x00, 0x00, //jne 0x0e
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_block_with_if_else_if_statement) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto ifchain = std::make_unique<IfChainStatement>();
    auto ifchainraw = ifchain.get();

    auto ifstatement = std::make_unique<IfStatement>();
    ifstatement->comparator = IfStatement::Equal;
    auto stackLhs = std::make_unique<StackVariableParam>();
    stackLhs->content = "test";
    ifstatement->lhs = std::move(stackLhs);
    auto int64Rhs = std::make_unique<Int64Param>();
    int64Rhs->content = 1234;
    ifstatement->rhs = std::move(int64Rhs);
    auto assign = std::make_unique<VariableAssignment>();
    assign->to.content = "test";
    auto valueParam = std::make_unique<Int64Param>();
    valueParam->content = 4;
    assign->value = std::move(valueParam);
    ifstatement->block = std::make_unique<Block>();
    ifstatement->block->statements.push_back(std::move(assign));
    ifstatement->block->parent = &block;
    ifchain->_ifstatements.push_back(std::move(ifstatement));

    auto ifstatement2 = std::make_unique<IfStatement>();
    ifstatement2->comparator = IfStatement::Equal;
    auto stackLhs2 = std::make_unique<StackVariableParam>();
    stackLhs2->content = "test";
    ifstatement2->lhs = std::move(stackLhs2);
    auto int64Rhs2 = std::make_unique<Int64Param>();
    int64Rhs2->content = 1234;
    ifstatement2->rhs = std::move(int64Rhs2);
    auto assign2 = std::make_unique<VariableAssignment>();
    assign2->to.content = "test";
    auto valueParam2 = std::make_unique<Int64Param>();
    valueParam2->content = 4;
    assign2->value = std::move(valueParam2);
    ifstatement2->block = std::make_unique<Block>();
    ifstatement2->block->statements.push_back(std::move(assign2));
    ifstatement2->block->parent = &block;
    ifchain->_ifstatements.push_back(std::move(ifstatement2));

    block.statements.push_back(std::move(ifchain));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_if_chain(ifchainraw);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            //if statement 1
            0x48, 0x8b, 0x45, 0xf8, //mov rax, [rbp - 0x8]
            0x48, 0xb9, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, 1234
            0x48, 0x3b, 0xc1, //cmp rax, rcx
            0x0f, 0x85, 0x13, 0x00, 0x00, 0x00, //jne 0x13
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8, //mov [rbp - 0x8], rax
            0xe9, 0x25, 0x00, 0x00, 0x00, //jmp 0x25
            //if statement 2, chained as else if
            0x48, 0x8b, 0x45, 0xf8, //mov rax, [rbp - 0x8]
            0x48, 0xb9, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, 1234
            0x48, 0x3b, 0xc1, //cmp rax, rcx
            0x0f, 0x85, 0x0e, 0x00, 0x00, 0x00, //jne 0x0e
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_block_with_if_else_statement) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto ifchain = std::make_unique<IfChainStatement>();
    auto ifchainraw = ifchain.get();

    auto ifstatement = std::make_unique<IfStatement>();
    ifstatement->comparator = IfStatement::Equal;
    auto stackLhs = std::make_unique<StackVariableParam>();
    stackLhs->content = "test";
    ifstatement->lhs = std::move(stackLhs);
    auto int64Rhs = std::make_unique<Int64Param>();
    int64Rhs->content = 1234;
    ifstatement->rhs = std::move(int64Rhs);
    auto assign = std::make_unique<VariableAssignment>();
    assign->to.content = "test";
    auto valueParam = std::make_unique<Int64Param>();
    valueParam->content = 4;
    assign->value = std::move(valueParam);
    ifstatement->block = std::make_unique<Block>();
    ifstatement->block->statements.push_back(std::move(assign));
    ifstatement->block->parent = &block;
    ifchain->_ifstatements.push_back(std::move(ifstatement));

    auto ifstatement2 = std::make_unique<IfStatement>();
    ifstatement2->comparator = IfStatement::None;
    auto assign2 = std::make_unique<VariableAssignment>();
    assign2->to.content = "test";
    auto valueParam2 = std::make_unique<Int64Param>();
    valueParam2->content = 4;
    assign2->value = std::move(valueParam2);
    ifstatement2->block = std::make_unique<Block>();
    ifstatement2->block->statements.push_back(std::move(assign2));
    ifstatement2->block->parent = &block;
    ifchain->_ifstatements.push_back(std::move(ifstatement2));

    block.statements.push_back(std::move(ifchain));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_if_chain(ifchainraw);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            //if statement 1
            0x48, 0x8b, 0x45, 0xf8, //mov rax, [rbp - 0x8]
            0x48, 0xb9, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, 1234
            0x48, 0x3b, 0xc1, //cmp rax, rcx
            0x0f, 0x85, 0x13, 0x00, 0x00, 0x00, //jne 0x0e
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8, //mov [rbp - 0x8], rax
            0xe9, 0x0e, 0x00, 0x00, 0x00, //jmp 0x25
            //if statement 2, chained as else if
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        })
    );
}

TEST(Compilerx64Tests, compile_block_with_loop_statement) {
    Block block;

    VariableDefinition def;
    def.name = "test";
    def.type = VariableDefinition::Int64;
    block.vars.push_back(def);

    auto loop = std::make_unique<LoopStatement>();
    auto loopRaw = loop.get();
    auto ifstatement = std::make_unique<IfStatement>();

    ifstatement->comparator = IfStatement::Equal;
    auto stackLhs = std::make_unique<StackVariableParam>();
    stackLhs->content = "test";
    ifstatement->lhs = std::move(stackLhs);
    auto int64Rhs = std::make_unique<Int64Param>();
    int64Rhs->content = 1234;
    ifstatement->rhs = std::move(int64Rhs);

    auto assign = std::make_unique<VariableAssignment>();
    assign->to.content = "test";
    
    auto valueParam = std::make_unique<Int64Param>();
    valueParam->content = 4;
    assign->value = std::move(valueParam);
    ifstatement->block = std::make_unique<Block>();
    ifstatement->block->statements.push_back(std::move(assign));
    ifstatement->block->parent = &block;
    loop->_ifStatement = std::move(ifstatement);
    block.statements.push_back(std::move(loop));

    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer);
    compiler.compile_loop(loopRaw);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0x8b, 0x45, 0xf8, //mov rax, [rbp - 0x8]
            0x48, 0xb9, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rcx, 1234
            0x48, 0x3b, 0xc1, //cmp rax, rcx
            0x0f, 0x85, 0x13, 0x00, 0x00, 0x00, //jne 0x13
            0x48, 0xb8, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8, //mov [rbp - 0x8], rax
            0xe9, 0xd6, 0xff, 0xff, 0xff
        })
    );
}
