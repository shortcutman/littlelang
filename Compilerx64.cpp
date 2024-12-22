
//------------------------------------------------------------------------------
// Compilerx64.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "Parser.hpp"
#include "InstrBufferx64.hpp"

#include <map>

using namespace compiler_x64;

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
        auto string = dynamic_cast<StringParam*>(call.params[i].get());
        if (string) {
            buff.mov_r64_imm64(index_to_register[i], reinterpret_cast<uint64_t>(const_cast<char*>(string->content.c_str())));
            continue;
        }

        auto intparam = dynamic_cast<Int64Param*>(call.params[i].get());
        if (intparam) {
            buff.mov_r64_imm64(index_to_register[i], intparam->content);
            continue;
        }

        auto stackparam = dynamic_cast<StackVariableParam*>(call.params[i].get());
        if (stackparam) {
            auto result = std::find_if(block.vars.begin(), block.vars.end(),
                [stackparam] (const VariableDefinition& def) { return def.name == stackparam->content;});
            if (result == block.vars.end()) {
                throw std::runtime_error("Unknown variable");
            }

            int8_t stackLocation = (std::distance(block.vars.begin(), result) + 1) * -8;
            buff.mov_r64_stack(index_to_register[i], stackLocation);
            continue;
        }

        throw std::runtime_error("Unknown parameter type.");
    }

    buff.call_r64(InstrBufferx64::Register::RAX);
}

void compiler_x64::compile_block_prefix(const ParsedBlock& block, InstrBufferx64& buff) {
    buff.push(InstrBufferx64::Register::RBP);
    buff.mov(InstrBufferx64::Register::RBP, InstrBufferx64::Register::RSP);

    auto stackSize = block.vars.size() * 8;
    auto remainder = stackSize % 16;
    if (remainder != 0) {
        stackSize += 16 - remainder;
    }

    buff.sub(InstrBufferx64::Register::RSP, stackSize);
}

void compiler_x64::compile_const_assignment(const ParsedBlock& block, const VariableConstAssignment& assignment, InstrBufferx64& buff) {
    //get stack mem loc
    std::optional<int8_t> stackLocation;
    for (size_t i = 0; i < block.vars.size(); i++) {
        if (block.vars[i].name == assignment.to) {
            stackLocation = (i + 1) * -8;
            break;
        }
    }

    if (stackLocation == std::nullopt) {
        throw std::runtime_error("can't find variable");
    }

    buff.mov_stack_imm64(*stackLocation, assignment.value);
}