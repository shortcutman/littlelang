
//------------------------------------------------------------------------------
// Compilerx64.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "Parser.hpp"
#include "InstrBufferx64.hpp"

#include <dlfcn.h>
#include <expected>
#include <map>
#include <ranges>
#include <sstream>

Compiler_x64::Compiler_x64(Block* block, InstrBufferx64* buff, Mode mode)
: _block(block)
, _buff(buff)
, _mode(mode)
{
}

void Compiler_x64::compile_function() {
    compile_function_prefix();
    compile_block();
    compile_function_suffix();
}

void Compiler_x64::compile_function_prefix() {
    _buff->push(InstrBufferx64::Register::RBP);
    _buff->mov_r64_r64(InstrBufferx64::Register::RBP, InstrBufferx64::Register::RSP);
}

void Compiler_x64::compile_function_suffix() {
    _buff->pop(InstrBufferx64::Register::RBP);
    _buff->ret();
}

void Compiler_x64::compile_block() {
    compile_block_prefix();

    for (auto& statement : _block->statements) {
        auto call = dynamic_cast<FunctionCall*>(statement.get());
        if (call != nullptr) {
            compile_function_call(*call);
            continue;
        }

        auto assign = dynamic_cast<VariableAssignment*>(statement.get());
        if (assign != nullptr) {
            compile_assignment(*assign);
            continue;
        }

        auto ifchain = dynamic_cast<IfChainStatement*>(statement.get());
        if (ifchain != nullptr) {
            compile_if_chain(ifchain);
            continue;
        }

        auto loop = dynamic_cast<LoopStatement*>(statement.get());
        if (loop != nullptr) {
            compile_loop(loop);
        }
    }

    compile_block_suffix();
}

void Compiler_x64::compile_function_call(const FunctionCall& call) {
    std::map<size_t, InstrBufferx64::Register> index_to_register({
        {0, InstrBufferx64::Register::RDI},
        {1, InstrBufferx64::Register::RSI},
        {2, InstrBufferx64::Register::RDX},
        {3, InstrBufferx64::Register::RCX}
    });

    if (call.params.size() > 4) {
        throw std::runtime_error("More than 4 function arguments not supported.");
    }

    for (size_t i = 0; i < call.params.size(); i++) {
        compile_parameter_to_register(call.params[i].get(), index_to_register[i]);
    }

    if (_mode == Mode::JIT) {
        void* dlHandle = dlopen(0, RTLD_NOW);
        void* functionAddr = dlsym(dlHandle, call.functionName.c_str());
        if (functionAddr) {
            _buff->mov_r64_imm64(
                InstrBufferx64::Register::RAX,
                reinterpret_cast<uint64_t>(functionAddr));
    
            _buff->call_r64(InstrBufferx64::Register::RAX);
        } else {
            _buff->call_rel32(0);
            _buff->_externFuncs.push_back({
                .symbol = call.functionName,
                .location = _buff->buffer().size() - sizeof(int32_t)
            });
        }
    } else if (_mode == Mode::ObjectFile) {
        _buff->call_rel32(0);
        _buff->_externFuncs.push_back({
            .symbol = call.functionName,
            .location = _buff->buffer().size() - sizeof(int32_t)
        });
    } else {
        throw std::runtime_error("unhandled mode");
    }
}

void Compiler_x64::compile_block_prefix() {
    auto stackSize = _block->stack_size_aligned();
    if (stackSize != 0) {
        _buff->sub(InstrBufferx64::Register::RSP, stackSize);
    }
}

void Compiler_x64::compile_block_suffix() {
    auto stackSize = _block->stack_size_aligned();
    if (stackSize != 0) {
        _buff->add_r64_imm32(InstrBufferx64::Register::RSP, stackSize);
    }
}

namespace {

size_t preceeding_block_sizes(Block& block) {
    if (block.parent) {
        return block.stack_size_aligned() + preceeding_block_sizes(*block.parent);
    } else {
        return 0;
    }
}

std::expected<int8_t, std::string> search_block(Block& block, const std::string& variable) {
    for (size_t i = 0; i < block.vars.size(); i++) {
        if (block.vars[i].name == variable) {
            return (i + 1) * -8 - preceeding_block_sizes(block);
        }
    }

    if (block.parent) {
        return search_block(*block.parent, variable);
    }

    std::stringstream ss;
    ss << "Cannot find variable name: " << variable;

    return std::unexpected(ss.str());
}

}

std::expected<int8_t, std::string> Compiler_x64::get_stack_location(const std::string& variable) {
    return search_block(*_block, variable);
}

void Compiler_x64::push_many_wo(std::vector<InstrBufferx64::Register> list, InstrBufferx64::Register skip) {
    for (auto reg : list) {
        if (reg != skip) {
            _buff->push(reg);
        }
    }
}

void Compiler_x64::pop_many_wo(std::vector<InstrBufferx64::Register> list, InstrBufferx64::Register skip) {
    for (auto reg : std::ranges::reverse_view{list}) {
        if (reg != skip) {
            _buff->pop(reg);
        }
    }
}

void Compiler_x64::compile_parameter_to_register(Param* param, InstrBufferx64::Register dest) {
    auto int64param = dynamic_cast<Int64Param*>(param);
    if (int64param) {
        _buff->mov_r64_imm64(dest, int64param->content);
        return;
    }

    auto stackvarparam = dynamic_cast<StackVariableParam*>(param);
    if (stackvarparam) {
        auto assignFromLocation = get_stack_location(stackvarparam->content).value();
        _buff->mov_r64_stack(dest, assignFromLocation);
        return;
    }

    auto string = dynamic_cast<StringParam*>(param);
    if (string) {
        bool imm64 = true;

#ifdef __linux__
        imm64 = _mode == Mode::ObjectFile ? false : true;
#endif

        if (imm64) {
            auto cstrAddr = _buff->add_cstring(string->content, _buff->buffer().size() + 2);
            _buff->mov_r64_imm64(dest, cstrAddr);
        } else {
            auto cstrAddr = _buff->add_cstring(string->content, _buff->buffer().size() + 3);
            _buff->lea_r64_riprel32(dest, 0);
        }
        
        return;
    }

    auto statementparam = dynamic_cast<StatementParam*>(param);
    if (statementparam) {
        auto int64calc = dynamic_cast<Int64Calcuation*>(statementparam->statement.get());
        if (int64calc) {
            switch (int64calc->operation) {
                case Int64Calcuation::Addition:
                {
                    auto destplus = static_cast<InstrBufferx64::Register>(static_cast<int>(dest) + 1);
                    push_many_wo({dest, destplus}, dest);

                    compile_parameter_to_register(int64calc->lhs.get(), dest);
                    compile_parameter_to_register(int64calc->rhs.get(), destplus);
                    _buff->add_r64_r64(dest, destplus);

                    pop_many_wo({dest, destplus}, dest);
                }
                    return;

                case Int64Calcuation::Modulo:
                {
                    push_many_wo({InstrBufferx64::Register::RAX, InstrBufferx64::Register::RDX, InstrBufferx64::Register::RCX}, dest);

                    compile_parameter_to_register(int64calc->lhs.get(), InstrBufferx64::Register::RAX);
                    compile_parameter_to_register(int64calc->rhs.get(), InstrBufferx64::Register::RCX);

                    _buff->cqo_idiv_r64(InstrBufferx64::Register::RCX);
                    _buff->mov_r64_r64(dest, InstrBufferx64::Register::RDX);

                    pop_many_wo({InstrBufferx64::Register::RAX, InstrBufferx64::Register::RDX, InstrBufferx64::Register::RCX}, dest);
                }
                    return;

                default:
                    throw std::runtime_error("unknown operation");
                    return;
            }
        } else {
            throw std::runtime_error("unknown statement");
        }
    }

    throw std::runtime_error("Unknown parameter type.");
}

void Compiler_x64::compile_assignment(const VariableAssignment& assignment) {
    auto assignToLocation = get_stack_location(assignment.to.content).value();

    compile_parameter_to_register(assignment.value.get(), InstrBufferx64::Register::RAX);
    
    _buff->mov_stack_r64(assignToLocation, InstrBufferx64::Register::RAX);
}

void Compiler_x64::compile_if_chain(IfChainStatement* chain) {
    std::vector<InstrBufferx64::JmpUpdate*> updates;

    for (size_t i = 0; i < chain->_ifstatements.size(); i++) {
        auto& ifStatement = chain->_ifstatements[i];
        InstrBufferx64 statementBuff;
        Compiler_x64 statementCompiler(ifStatement->block.get(), &statementBuff, _mode);
        statementCompiler.compile_block();
        if (i != (chain->_ifstatements.size() - 1)) {
            updates.push_back(statementBuff.jmp_with_update());
        }
        auto blockSize = statementBuff.buffer().size();

        if (ifStatement->comparator != IfStatement::None) {
            compile_comparator(ifStatement.get(), blockSize);
        }
        
        _buff->append_buffer(statementBuff);
    }

    auto chainEnd = _buff->buffer().size();
    for (auto update : updates) {
        _buff->update_jmp(update, chainEnd - update->location);
    }
}

void Compiler_x64::compile_loop(LoopStatement* loop) {
    InstrBufferx64::JmpUpdate* update = nullptr;
    auto& ifStatement = loop->_ifStatement;
    size_t beforeLoopStatementSize = _buff->buffer().size();
    
    InstrBufferx64 statementBuff;
    Compiler_x64 statementCompiler(ifStatement->block.get(), &statementBuff, _mode);
    statementCompiler.compile_block();
    update = statementBuff.jmp_with_update();
    auto blockSize = statementBuff.buffer().size();

    compile_comparator(ifStatement.get(), blockSize);
    
    _buff->append_buffer(statementBuff);
    _buff->update_jmp(update, beforeLoopStatementSize - _buff->buffer().size());
}

void Compiler_x64::compile_comparator(IfStatement* comparison, int32_t offset) {
    compile_parameter_to_register(comparison->lhs.get(), InstrBufferx64::Register::RAX);
    compile_parameter_to_register(comparison->rhs.get(), InstrBufferx64::Register::RCX);
    _buff->cmp(InstrBufferx64::Register::RAX, InstrBufferx64::Register::RCX);

    if (comparison->comparator == IfStatement::Equal) {
        _buff->jmp_not_equal(offset);
    } else if (comparison->comparator == IfStatement::LessThan) {
        _buff->jmp_greater_or_equal(offset);
    } else {
        throw std::runtime_error("unhandled comparator");
    }
}
