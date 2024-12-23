
//------------------------------------------------------------------------------
// Compilerx64.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "Parser.hpp"
#include "InstrBufferx64.hpp"

#include <expected>
#include <map>
#include <ranges>
#include <sstream>

using namespace compiler_x64;

void compiler_x64::compile_block(const ParsedBlock& block, InstrBufferx64& buff) {
    compile_block_prefix(block, buff);

    for (auto& statement : block.statements) {
        auto call = dynamic_cast<FunctionCall*>(statement.get());
        if (call != nullptr) {
            compile_function_call(block, *call, buff);
            continue;
        }

        auto assign = dynamic_cast<VariableAssignment*>(statement.get());
        if (assign != nullptr) {
            compile_assignment(block, *assign, buff);
            continue;
        }
    }

    compile_block_suffix(block, buff);
}

void compiler_x64::compile_function_call(const ParsedBlock& block, const FunctionCall& call, InstrBufferx64& buff) {
    buff.mov_r64_imm64(
        InstrBufferx64::Register::RAX,
        reinterpret_cast<uint64_t>(call.functionAddr));

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
        compile_parameter_to_register(block, call.params[i].get(), index_to_register[i], buff);
    }

    buff.call_r64(InstrBufferx64::Register::RAX);
}

void compiler_x64::compile_block_prefix(const ParsedBlock& block, InstrBufferx64& buff) {
    buff.push(InstrBufferx64::Register::RBP);
    buff.mov_r64_r64(InstrBufferx64::Register::RBP, InstrBufferx64::Register::RSP);

    int32_t stackSize = block.vars.size() * 8;
    auto remainder = stackSize % 16;
    if (remainder != 0) {
        stackSize += 16 - remainder;
    }

    if (stackSize != 0) {
        buff.sub(InstrBufferx64::Register::RSP, stackSize);
    }
}

void compiler_x64::compile_block_suffix(const ParsedBlock& block, InstrBufferx64& buff) {
    int32_t stackSize = block.vars.size() * 8;
    auto remainder = stackSize % 16;
    if (remainder != 0) {
        stackSize += 16 - remainder;
    }

    if (stackSize != 0) {
        buff.add_r64_imm32(InstrBufferx64::Register::RSP, stackSize);
    }
    
    buff.pop(InstrBufferx64::Register::RBP);
    buff.ret();
}

namespace {
    std::expected<int8_t, std::string> get_stack_location(const ParsedBlock& block, const std::string& variable) {
        for (size_t i = 0; i < block.vars.size(); i++) {
            if (block.vars[i].name == variable) {
                return (i + 1) * -8;
            }
        }

        std::stringstream ss;
        ss << "Cannot find variable name: " << variable;

        return std::unexpected(ss.str());
    }

    void push_many_wo(InstrBufferx64& buff, std::vector<InstrBufferx64::Register> list, InstrBufferx64::Register skip) {
        for (auto reg : list) {
            if (reg != skip) {
                buff.push(reg);
            }
        }
    }

    void pop_many_wo(InstrBufferx64& buff, std::vector<InstrBufferx64::Register> list, InstrBufferx64::Register skip) {
        for (auto reg : std::ranges::reverse_view{list}) {
            if (reg != skip) {
                buff.pop(reg);
            }
        }
    }
}

void compiler_x64::compile_parameter_to_register(const ParsedBlock& block, Param* param, InstrBufferx64::Register dest, InstrBufferx64& buff) {
    auto int64param = dynamic_cast<Int64Param*>(param);
    if (int64param) {
        buff.mov_r64_imm64(dest, int64param->content);
        return;
    }

    auto stackvarparam = dynamic_cast<StackVariableParam*>(param);
    if (stackvarparam) {
        auto assignFromLocation = get_stack_location(block, stackvarparam->content);
        buff.mov_r64_stack(dest, assignFromLocation.value());
        return;
    }

    auto string = dynamic_cast<StringParam*>(param);
    if (string) {
        buff.mov_r64_imm64(dest, reinterpret_cast<uint64_t>(const_cast<char*>(string->content.c_str())));
        return;
    }

    auto statementparam = dynamic_cast<StatementParam*>(param);
    if (statementparam) {
        auto int64calc = dynamic_cast<Int64Calcuation*>(statementparam->statement.get());
        if (int64calc) {

            //TODO: This clobbers the RAX and RCX registers! push them to the stack!!
            //or maybe push that burden to the caller

            switch (int64calc->operation) {
                case Int64Calcuation::Addition:
                {
                    auto destplus = static_cast<InstrBufferx64::Register>(static_cast<int>(dest) + 1);
                    compile_parameter_to_register(block, int64calc->lhs.get(), dest, buff);
                    compile_parameter_to_register(block, int64calc->rhs.get(), destplus, buff);
                    buff.add_r64_r64(dest, destplus);
                }
                    return;

                case Int64Calcuation::Modulo:
                {
                    push_many_wo(buff, {InstrBufferx64::Register::RAX, InstrBufferx64::Register::RDX, InstrBufferx64::Register::RCX}, dest);

                    compile_parameter_to_register(block, int64calc->lhs.get(), InstrBufferx64::Register::RAX, buff);
                    compile_parameter_to_register(block, int64calc->rhs.get(), InstrBufferx64::Register::RCX, buff);

                    buff.cqo_idiv_r64(InstrBufferx64::Register::RCX);
                    buff.mov_r64_r64(dest, InstrBufferx64::Register::RDX);

                    pop_many_wo(buff, {InstrBufferx64::Register::RAX, InstrBufferx64::Register::RDX, InstrBufferx64::Register::RCX}, dest);
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

void compiler_x64::compile_assignment(const ParsedBlock& block, const VariableAssignment& assignment, InstrBufferx64& buff) {
    auto assignToLocation = get_stack_location(block, assignment.to.content);

    compile_parameter_to_register(block, assignment.value.get(), InstrBufferx64::Register::RAX, buff);
    
    buff.mov_stack_r64(assignToLocation.value(), InstrBufferx64::Register::RAX);
}