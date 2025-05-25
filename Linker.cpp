
//------------------------------------------------------------------------------
// Linker.cpp
//------------------------------------------------------------------------------

#include "Linker.hpp"

#include "TranslationUnit.hpp"

#include <map>

ll::Object ll::Object::compile_translation_unit(const TranslationUnit& tu, Compiler_x64::Mode mode) {
    ll::Object obj;

    std::map<std::string, std::size_t> symbols;

    for (auto& func : tu.functions) {
        obj.symbols.push_back(Symbol{
            .name = func->name,
            .offset = obj.buff.buffer().size()
        });

        symbols.insert({func->name, obj.buff.buffer().size()});

        auto compiler = Compiler_x64(func->block.get(), &obj.buff, mode);
        compiler.compile_function();
    }

    decltype(obj.buff._externFuncs) filteredExternFuncs;
    for (auto& extFn : obj.buff._externFuncs) {
        auto sym = symbols.find(extFn.symbol);
        if (sym != symbols.end()) {
            int32_t* relocationAddr = reinterpret_cast<int32_t*>(&obj.buff.buffer()[extFn.location]);
            *relocationAddr = sym->second - (extFn.location + 4);
        } else {
            filteredExternFuncs.push_back(extFn);
        }
    }
    obj.buff._externFuncs = filteredExternFuncs;

    return obj;
}