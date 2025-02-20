
//------------------------------------------------------------------------------
// Compilerx64.hpp
//------------------------------------------------------------------------------

#pragma once

#include "InstrBufferx64.hpp"
#include "Statement.hpp"

#include <expected>

class Compiler_x64 {

public:
    enum class Mode {
        JIT,
        ObjectFile
    };

private:
    Block* _block = nullptr;
    InstrBufferx64* _buff = nullptr;
    Mode _mode = Mode::JIT;

public:
    Compiler_x64(Block* block, InstrBufferx64* buff, Mode mode = Mode::JIT);

    void compile_function();
    void compile_block();

    void compile_function_prefix();
    void compile_block_prefix();
    void compile_assignment(const VariableAssignment& assignment);
    void compile_function_call(const FunctionCall& call);
    void compile_parameter_to_register(Param* param, InstrBufferx64::Register dest);
    void compile_if_chain(IfChainStatement* chain);
    void compile_loop(LoopStatement* loop);
    void compile_comparator(IfStatement* comparison, int32_t offset);
    void compile_block_suffix();
    void compile_function_suffix();

    std::expected<int8_t, std::string> get_stack_location(const std::string& variable);
    
    void push_many_wo(std::vector<InstrBufferx64::Register> list, InstrBufferx64::Register skip);
    void pop_many_wo(std::vector<InstrBufferx64::Register> list, InstrBufferx64::Register skip);
};
