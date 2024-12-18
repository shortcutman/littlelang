
//------------------------------------------------------------------------------
// Compilerx64.tests.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "InstrBufferx64.hpp"
#include "Parser.hpp"

#include <gtest/gtest.h>

TEST(Compilerx64Tests, compile_function_call) {
    FunctionCall call{
        "testFunc",
        reinterpret_cast<void*>(0x1122334455667788),
        reinterpret_cast<void*>(0xaabbccddeeffaabb)
    };
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