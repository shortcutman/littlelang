
//------------------------------------------------------------------------------
// Compilerx64.hpp
//------------------------------------------------------------------------------

#pragma once

struct FunctionCall;
class InstrBufferx64;
class ParsedBlock;

namespace compiler_x64 {

    void compile_function_call(const FunctionCall& call, InstrBufferx64& buff);
    void compile_block_prefix(const ParsedBlock& block, InstrBufferx64& buff);

}