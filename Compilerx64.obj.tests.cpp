
//------------------------------------------------------------------------------
// Compilerx64.obj.tests.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include <gtest/gtest.h>

TEST(Compilerx64ObjectTests, compile_function_call) {
    FunctionCall call;
    call.functionName = "puts";

    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer, Compiler_x64::Mode::ObjectFile);
    compiler.compile_function_call(call);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0xe8, 0x00, 0x00, 0x00, 0x00 //call $rip + 0
        }));

    EXPECT_EQ(
        compiler._externFuncs.front(),
        (Compiler_x64::ExternFunction{
            .symbol = "puts",
            .location = 1
        })
    );
}

TEST(Compilerx64ObjectTests, compile_function_call_intparam) {
    FunctionCall call;
    call.functionName = "puts";

    auto intparam = std::make_unique<Int64Param>();
    intparam->content = 0xaabbccddeeffaabb;
    call.params.push_back(std::move(intparam));

    Block block;
    InstrBufferx64 buffer;
    auto compiler = Compiler_x64(&block, &buffer, Compiler_x64::Mode::ObjectFile);
    compiler.compile_function_call(call);

    EXPECT_EQ(
        buffer.buffer(),
        std::vector<uint8_t>({
            0x48, 0xbf, 0xbb, 0xaa, 0xff, 0xee, 0xdd, 0xcc, 0xbb, 0xaa, //mov rdi, imm64
            0xe8, 0x00, 0x00, 0x00, 0x00 //call $rip + 0
        }));

    EXPECT_EQ(
        compiler._externFuncs.front(),
        (Compiler_x64::ExternFunction{
            .symbol = "puts",
            .location = 11
        })
    );
}
