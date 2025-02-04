
//------------------------------------------------------------------------------
// Elf.cpp
//------------------------------------------------------------------------------

#include "Elf.hpp"

#include "InstrBufferx64.hpp"

#include <cstring>

namespace {
    template<typename T>
    void write_os(std::ostream& os, T* from, size_t count) {
        os.write(reinterpret_cast<std::ostream::char_type*>(from), count);
    }

    template<typename T>
    void bytes_to_vec(std::vector<uint8_t>& vec, T* source, size_t amount) {
        uint8_t* start = reinterpret_cast<uint8_t*>(const_cast<std::remove_const<T>::type*>( source));
        vec.insert(vec.end(), start, start + amount);
    }

    void string_to_vec(std::vector<uint8_t>& vec, std::string& source) {
        vec.insert(vec.end(), source.begin(), source.end());
        vec.push_back(0x00);
    }

    void string_to_vec(std::vector<uint8_t>& vec, const char* source) {
        vec.insert(vec.end(), source, source + strlen(source));
        vec.push_back(0x00);
    }

    template<typename T>
    uint32_t offset_and_insert(std::vector<uint8_t>& vec, T source) {
        auto offset = vec.size();
        string_to_vec(vec, source);
        return offset;
    }
}

elf::RelocationData elf::RelocationData::generate(InstrBufferx64& instrs) {
    RelocationData rd;
    rd.strtab.push_back(0x00);

    //null symbol
    rd.symbols.push_back(SymbolEntry{
        .name = offset_and_insert(rd.strtab, ""),
        .info = 0,
        .other = 0,
        .shndx = 0,
        .value = 0,
        .size = 0,
    });

    rd.symbols.push_back(SymbolEntry{
        .name = offset_and_insert(rd.strtab, "out.o"),
        .info = (0x4) /*STT_FILE*/,
        .other = 0,
        .shndx = 0xfff1,
        .value = 0,
        .size = 0,
    });

    //.text symbol
    rd.symbols.push_back(SymbolEntry{
        .name = offset_and_insert(rd.strtab, ".text"),
        .info = (0x00 << 4 /*STB_LOCAL*/) | (0x03 /*STT_SECTION*/),
        .other = 0,
        .shndx = 1,
        .value = 0,
        .size = 0,
    });

    //rodata symbol
    rd.symbols.push_back(SymbolEntry{
        .name = offset_and_insert(rd.strtab, ".rodata"), //offset into shstrtab
        .info = (0x00 << 4 /* STB_LOCAL */) | (0x03 /* STT_SECTION */),
        .shndx = 3, // section header index
        .value = 0,
        .size = 0,
    });

    for (auto& strReloc : instrs._cstrings) {
        rd.relocs.push_back(RelocationEntry{
            .offset = strReloc->location,
            .type = 0x02, // R_X86_64_PC32
            // .type = 0x01, //R_X86_64_64
            .symbol = static_cast<uint32_t>(rd.symbols.size() - 1),
            .addend = (static_cast<int64_t>(offset_and_insert(rd.rodata, strReloc->string)) - 4) // -4 since PC always points to next instr
        });
    }

    rd.symbols.push_back(SymbolEntry{
        .name = offset_and_insert(rd.strtab, "main"),
        .info = (0x01 << 4 /* STB_GLOBAL */) | (0x02 /* STT_FUNC */),
        .shndx = 1, // section header index
        .value = 0,
        .size = instrs.buffer().size(),
    });

    for (auto& extReloc : instrs._externFuncs) {
        rd.symbols.push_back(SymbolEntry{
            .name = offset_and_insert(rd.strtab, extReloc.symbol),
            .info = (0x01 << 4 /*STB_GLOBAL*/) | (0x00 /*STT_NOTYPE*/),
            .shndx = 0,
            .value = 0,
            .size = 0
        });

        rd.relocs.push_back(RelocationEntry{
            .offset = extReloc.location,
            .type = 0x04, /*R_X86_64_PLT32*/
            .symbol = static_cast<uint32_t>(rd.symbols.size() - 1),
            .addend = -4
        });
    }

    return rd;
}

void elf::write(std::ostream& out, InstrBufferx64& buff) {
    std::vector<uint8_t> data;
    std::vector<uint8_t> shstrtab_data;
    shstrtab_data.push_back(0x00);

    auto relocation_data = RelocationData::generate(buff);

    elf::Header elfheader;

    elf::SectionHeader nullsection;

    elf::SectionHeader text;
    text.sh_name = offset_and_insert(shstrtab_data, ".text");
    text.sh_type = 0x1; //SHT_PROGBITS
    text.sh_flags = 0x2 /* SHF_ALLOC */ | 0x4 /* SHF_EXECINSTR */;
    text.sh_offset = data.size() + sizeof(elfheader);
    text.sh_size = buff.buffer().size();
    text.sh_addralign = 1;
    bytes_to_vec(data, &buff.buffer()[0], buff.buffer().size());

    elf::SectionHeader relatext;
    relatext.sh_name = offset_and_insert(shstrtab_data, ".rela.text");
    relatext.sh_type = 0x4; //SHT_RELA
    relatext.sh_flags = 0x40 /* SHF_INFO_LINK */;
    relatext.sh_offset = data.size() + sizeof(elfheader);
    relatext.sh_size = relocation_data.relocs.size() * sizeof(RelocationEntry);
    relatext.sh_link = 4; //symtab shidx
    relatext.sh_info = 1; //.text shidx
    relatext.sh_addralign = 8; // 2^3 = 8
    relatext.sh_entsize = sizeof(RelocationEntry);
    bytes_to_vec(data, &relocation_data.relocs[0], relatext.sh_size);

    elf::SectionHeader rodata;
    rodata.sh_name = offset_and_insert(shstrtab_data, ".rodata");
    rodata.sh_type = 0x1; //SHT_PROGBITS
    rodata.sh_flags = 0x2 /* SHF_ALLOC */;
    rodata.sh_offset = data.size() + sizeof(elfheader);
    rodata.sh_size = relocation_data.rodata.size();
    rodata.sh_addralign = 1;
    bytes_to_vec(data, &relocation_data.rodata[0], rodata.sh_size);

    elf::SectionHeader symtab;
    symtab.sh_name = offset_and_insert(shstrtab_data, ".symtab");
    symtab.sh_type = 0x2; //SHT_SYMTAB
    symtab.sh_flags = 0x0;
    symtab.sh_offset = data.size() + sizeof(elfheader);
    symtab.sh_size = relocation_data.symbols.size() * sizeof(SymbolEntry);
    symtab.sh_link = 5; //strtab shidx
    symtab.sh_info = 4; //one greater than the last LOCAL symbol table index
    symtab.sh_entsize = sizeof(SymbolEntry);
    symtab.sh_addralign = 8; // 2^3 = 8
    bytes_to_vec(data, &relocation_data.symbols[0], symtab.sh_size);

    elf::SectionHeader strtab;
    strtab.sh_name = offset_and_insert(shstrtab_data, ".strtab");
    strtab.sh_type = 0x3; //SHT_STRTAB
    strtab.sh_flags = 0x0;
    strtab.sh_offset = data.size() + sizeof(elfheader);
    strtab.sh_size = relocation_data.strtab.size();
    strtab.sh_addralign = 1;
    bytes_to_vec(data, &relocation_data.strtab[0], strtab.sh_size);

    elf::SectionHeader shstrtab;
    shstrtab.sh_name = offset_and_insert(shstrtab_data, ".shstrtab");
    shstrtab.sh_type = 0x3; //SHT_STRTAB
    shstrtab.sh_flags = 0 /* No flags */;
    shstrtab.sh_offset = data.size() + sizeof(elfheader);
    shstrtab.sh_size = shstrtab_data.size();
    shstrtab.sh_addralign = 1;
    bytes_to_vec(data, &shstrtab_data[0], shstrtab_data.size());

    elfheader.e_shoff = data.size() + sizeof(elfheader);
    elfheader.e_shentsize = sizeof(SectionHeader);
    elfheader.e_shnum = 7;
    elfheader.e_shstrndx = 6;
    write_os(out, &elfheader, sizeof(elfheader));

    write_os(out, &data[0], data.size());

    write_os(out, &nullsection, sizeof(SectionHeader));
    write_os(out, &text, sizeof(SectionHeader));
    write_os(out, &relatext, sizeof(SectionHeader));
    write_os(out, &rodata, sizeof(SectionHeader));
    write_os(out, &symtab, sizeof(SectionHeader));
    write_os(out, &strtab, sizeof(SectionHeader));
    write_os(out, &shstrtab, sizeof(SectionHeader));
}