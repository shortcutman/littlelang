
//------------------------------------------------------------------------------
// Compilerx64.tests.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "InstrBufferx64.hpp"
#include "Parser.hpp"

#include <gtest/gtest.h>

TEST(Compilerx64Tests, compile_function_call_with_intparam) {
    FunctionCall call;
    call.functionName = "testFunc";
    call.functionAddr = reinterpret_cast<void*>(0x1122334455667788);

    auto intparam = std::make_unique<Int64Param>();
    intparam->content = 0xaabbccddeeffaabb;
    call.params.push_back(std::move(intparam));

    InstrBufferx64 buffer;
    compiler_x64::compile_function_call(call, buffer);

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

    InstrBufferx64 buffer;
    compiler_x64::compile_function_call(call, buffer);

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

    InstrBufferx64 buffer;
    compiler_x64::compile_function_call(call, buffer);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, //mov rax, imm64
            0x48, 0xbf, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
            0x48, 0xbe, 0xd2, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
            0xff, 0xd0 //call rax
        }));
}