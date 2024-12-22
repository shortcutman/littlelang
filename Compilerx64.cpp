
//------------------------------------------------------------------------------
// Compilerx64.cpp
//------------------------------------------------------------------------------

#include "Compilerx64.hpp"

#include "Parser.hpp"
#include "InstrBufferx64.hpp"

#include <map>

using namespace compiler_x64;

void compiler_x64::compile_function_call(const FunctionCall& call, InstrBufferx64& buff) {
    buff.push_mov_r64_imm64(
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
            buff.push_mov_r64_imm64(index_to_register[i], reinterpret_cast<uint64_t>(const_cast<char*>(string->content.c_str())));
            continue;
        }

        auto intparam = dynamic_cast<Int64Param*>(call.params[i].get());
        if (intparam) {
            buff.push_mov_r64_imm64(index_to_register[i], intparam->content);
            continue;
        }

        throw std::runtime_error("Unknown parameter type.");
    }

    buff.call_r64(InstrBufferx64::Register::RAX);
}