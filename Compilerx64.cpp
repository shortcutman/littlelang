
//------------------------------------------------------------------------------
// Compilerx64.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "Parser.hpp"
#include "InstrBufferx64.hpp"

using namespace compiler_x64;

void compiler_x64::compile_function_call(const FunctionCall& call, InstrBufferx64& buff) {
    buff.push_mov_r64_imm64(
        InstrBufferx64::Register::RAX,
        reinterpret_cast<uint64_t>(call.functionAddr));
    buff.push_mov_r64_imm64(
        InstrBufferx64::Register::RDI,
        reinterpret_cast<uint64_t>(call.param));
    buff.call_r64(InstrBufferx64::Register::RAX);
}