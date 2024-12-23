
//------------------------------------------------------------------------------
// Compilerx64.hpp
//------------------------------------------------------------------------------

#pragma once

struct FunctionCall;
class ParsedBlock;
class Param;
class VariableAssignment;

#include "InstrBufferx64.hpp"

namespace compiler_x64 {

    void compile_block(const ParsedBlock& block, InstrBufferx64& buff);

    void compile_block_prefix(const ParsedBlock& block, InstrBufferx64& buff);
    void compile_assignment(const ParsedBlock& block, const VariableAssignment& assignment, InstrBufferx64& buff);
    void compile_function_call(const ParsedBlock& block, const FunctionCall& call, InstrBufferx64& buff);
    void compile_parameter_to_register(const ParsedBlock& block, Param* param, InstrBufferx64::Register dest, InstrBufferx64& buff);
    void compile_block_suffix(const ParsedBlock& block, InstrBufferx64& buff);
}