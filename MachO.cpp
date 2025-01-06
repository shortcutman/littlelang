
//------------------------------------------------------------------------------
// MachO.cpp
//------------------------------------------------------------------------------

#include "MachO.hpp"

#include "InstrBufferx64.hpp"

namespace {
    template<typename T>
    void bytes_to_vec(std::vector<uint8_t>& vec, T* source, size_t amount) {
        uint8_t* start = reinterpret_cast<uint8_t*>(const_cast<std::remove_const<T>::type*>( source));
        vec.insert(vec.end(), start, start + amount);
    }
}

macho::SymbolData macho::SymbolData::generate(InstrBufferx64& buff) {
    SymbolData data;
    std::vector<uint8_t> symbols;
    std::vector<uint8_t> strings;
    strings.push_back(0x00);

    //start with main
    Symbol main_sym{
        .nameoffset = static_cast<uint32_t>(strings.size()),
        .symboltype = 0x0f,
        .sectionno = 1,
        .datainfo = 0x0000,
        .symboladdress = 0x0000
    };
    const char* main_name = "_main";
    bytes_to_vec(strings, main_name, strlen(main_name) + 1);
    bytes_to_vec(symbols, &main_sym, sizeof(main_sym));
    data._name_to_index[main_name] = 0;

    for (auto ext : buff._externFuncs) {
        if (data._name_to_index.contains(ext.symbol)) {
            continue;
        }

        Symbol sym{
            .nameoffset = static_cast<uint32_t>(strings.size()),
            .symboltype = 0x01,
            .sectionno = 0,
            .datainfo = 0x0000,
            .symboladdress = 0x00
        };
        data._name_to_index[ext.symbol] = data._name_to_index.size();
        std::string externString = "_" + ext.symbol;
        bytes_to_vec(strings, externString.c_str(), externString.size() + 1);
        bytes_to_vec(symbols, &sym, sizeof(sym));
    }

    data._data.insert(data._data.end(), symbols.begin(), symbols.end());
    data._data.insert(data._data.end(), strings.begin(), strings.end());
    
    return data;
}

macho::CStringData macho::CStringData::generate(InstrBufferx64& buff) {
    CStringData data;

    for (auto& cstr : buff._cstrings) {
        if (data._string_to_offset.contains(cstr->string)) {
            continue;
        }

        data._string_to_offset[cstr->string] = data._data.size();
        bytes_to_vec(data._data, cstr->string.c_str(), cstr->string.size() + 1);
    }

    return data;
}

std::vector<uint8_t> macho::create_reloc_data(
    InstrBufferx64& instrs,
    std::vector<uint8_t>& buff,
    const CStringData& cstrings,
    const SymbolData& symbols) {
    struct RelocationEntry {
        int32_t address;
        uint32_t flags;
    };
    std::vector<uint8_t> relocations;

    for (auto& strReloc : instrs._cstrings) {
        auto cstrDataOffset = cstrings._string_to_offset.find(strReloc->string);
        if (cstrDataOffset == cstrings._string_to_offset.end()) {
            throw std::runtime_error("Cstring missing");
        }

        //location is the address bytes for the string
        //needs to be replaced with offset
        uint64_t offset = cstrDataOffset->second;
        uint64_t* replaceLocation = reinterpret_cast<uint64_t*>(&buff[strReloc->location]);
        *replaceLocation = buff.size() + offset;

        RelocationEntry reloc{
            .address = static_cast<int32_t>(strReloc->location),
            .flags = 0x06000002
        };
        bytes_to_vec(relocations, &reloc, sizeof(reloc));
    }

    for (auto& extReloc : instrs._externFuncs) {
        auto extSym = symbols._name_to_index.find(extReloc.symbol);
        if (extSym == symbols._name_to_index.end()) {
            throw std::runtime_error("Extern symbol missing");
        }

        RelocationEntry reloc{
            .address = static_cast<int32_t>(extReloc.location),
            .flags = 0x2d000000 | (extSym->second & 0xffffff)
        };
        bytes_to_vec(relocations, &reloc, sizeof(reloc));
    }

    return relocations;
}
