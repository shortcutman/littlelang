
//------------------------------------------------------------------------------
// Compilerx64.hpp
//------------------------------------------------------------------------------

#pragma once

struct FunctionCall;
class InstrBufferx64;

namespace compiler_x64 {

    void compile_function_call(const FunctionCall& call, InstrBufferx64& buff);

}