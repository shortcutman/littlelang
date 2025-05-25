
//------------------------------------------------------------------------------
// Linker.hpp
//------------------------------------------------------------------------------

#pragma once

#include "InstrBufferx64.hpp"

#include "Compilerx64.hpp"

namespace ll {

class TranslationUnit;

struct Symbol {
    std::string name;
    std::size_t offset;
    auto operator<=>(const Symbol&) const = default;
};

struct Object {
    InstrBufferx64 buff;
    std::vector<Symbol> symbols;

    static Object compile_translation_unit(const TranslationUnit& tu, Compiler_x64::Mode mode);
};

}