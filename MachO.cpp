
//------------------------------------------------------------------------------
// MachO.cpp
//------------------------------------------------------------------------------

#include "MachO.hpp"

#include "InstrBufferx64.hpp"

namespace {
    void populate_text(std::string in, char* out) {
        if (in.size() > 16) {
            throw std::runtime_error("input too big for macho load segments");
        }

        std::fill_n(out, 16, 0x00);
        std::copy(in.begin(), in.end(), out);
    }

    template<typename T>
    void write_os(std::ostream& os, T* from, size_t count) {
        os.write(reinterpret_cast<std::ostream::char_type*>(from), count);
    }

    template<typename T>
    void bytes_to_vec(std::vector<uint8_t>& vec, T* source, size_t amount) {
        uint8_t* start = reinterpret_cast<uint8_t*>(const_cast<std::remove_const<T>::type*>( source));
        vec.insert(vec.end(), start, start + amount);
    }
}

macho::SymbolTable macho::SymbolData::symbol_table(size_t symbols_offset) const {
    return macho::SymbolTable{
        .symbolsoffset = static_cast<uint32_t>(symbols_offset),
        .numberofsymbols = static_cast<uint32_t>(this->_name_to_index.size()),
        .stringtableoffset = static_cast<uint32_t>(symbols_offset + this->_string_table_offset),
        .stringtablesize = static_cast<uint32_t>(this->_string_table_size)
    };
}

macho::DynamicSymbolTable macho::SymbolData::dynamic_symbol_table() const {
    return macho::DynamicSymbolTable{
        .iextdefsym = 0,
        .nextdefsym = 1, //referencing main
        .iundefsym = static_cast<uint32_t>(this->_name_to_index.size() - 1),
    };
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

    data._string_table_offset = symbols.size();
    data._string_table_size = strings.size();

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

void macho::write(std::ostream& out, InstrBufferx64& buff) {
    auto cstrings = CStringData::generate(buff);
    auto symbols = SymbolData::generate(buff);
    std::vector<uint8_t> instruction_data = buff.buffer();
    auto relocation_data = create_reloc_data(buff, instruction_data, cstrings, symbols);

    macho::Header machoheader;

    macho::LoadSegment64 load_segment_64;
    macho::SegmentSection64 segment_section_text;
    macho::SegmentSection64 segment_section_cstring;
    macho::BuildVersion build_version;
    macho::SymbolTable symtab;
    macho::DynamicSymbolTable dsymtab;

    machoheader.sizeofload = sizeof(load_segment_64) +
        sizeof(segment_section_text) +
        sizeof(segment_section_cstring) +
        sizeof(build_version) +
        sizeof(symtab) +
        sizeof(dsymtab);
    write_os(out, &machoheader, sizeof(machoheader));
    
    populate_text("__TEXT", load_segment_64.segmentname);
    load_segment_64.addresssize = instruction_data.size() + cstrings._data.size();
    load_segment_64.fileoffset = sizeof(machoheader) + machoheader.sizeofload;
    load_segment_64.filesize = load_segment_64.addresssize;
    write_os(out, &load_segment_64, sizeof(load_segment_64));

    populate_text("__text", segment_section_text.sectionname);
    populate_text("__TEXT", segment_section_text.segmentname);
    segment_section_text.address = 0;
    segment_section_text.addresssize = instruction_data.size();
    segment_section_text.fileoffset = load_segment_64.fileoffset;
    segment_section_text.relocationsfileoff = load_segment_64.fileoffset + load_segment_64.filesize;
    segment_section_text.numberofrelocations = 2;
    write_os(out, &segment_section_text, sizeof(segment_section_text));

    populate_text("__cstring", segment_section_cstring.sectionname);
    populate_text("__TEXT", segment_section_cstring.segmentname);
    segment_section_cstring.address = segment_section_text.addresssize;
    segment_section_cstring.addresssize = cstrings._data.size();
    segment_section_cstring.fileoffset = load_segment_64.fileoffset + segment_section_text.addresssize;
    write_os(out, &segment_section_cstring, sizeof(segment_section_cstring));

    write_os(out, &build_version, sizeof(build_version));

    symtab = symbols.symbol_table(load_segment_64.fileoffset + segment_section_text.addresssize + segment_section_cstring.addresssize + 16); //+16 = relocs
    write_os(out, &symtab, sizeof(symtab));

    dsymtab = symbols.dynamic_symbol_table();
    write_os(out, &dsymtab, sizeof(dsymtab));

    write_os(out, &instruction_data[0], instruction_data.size());
    write_os(out, &cstrings._data[0], cstrings._data.size());
    write_os(out, &relocation_data[0], relocation_data.size());
    write_os(out, &symbols._data[0], symbols._data.size());
}