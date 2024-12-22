
//------------------------------------------------------------------------------
// Compilerx64.hpp
//------------------------------------------------------------------------------

#pragma once

struct FunctionCall;
class InstrBufferx64;
class ParsedBlock;
class VariableAssignment;

namespace compiler_x64 {

    void compile_block(const ParsedBlock& block, InstrBufferx64& buff);

    void compile_block_prefix(const ParsedBlock& block, InstrBufferx64& buff);
    void compile_assignment(const ParsedBlock& block, const VariableAssignment& assignment, InstrBufferx64& buff);
    void compile_function_call(const ParsedBlock& block, const FunctionCall& call, InstrBufferx64& buff);
    void compile_block_suffix(const ParsedBlock& block, InstrBufferx64& buff);

}