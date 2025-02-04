
//------------------------------------------------------------------------------
// Elf.hpp
//------------------------------------------------------------------------------

#pragma once

class InstrBufferx64;

#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

namespace elf {
    void write(std::ostream& out, InstrBufferx64& buff);

    struct Header {
        uint32_t magic = 0x464c457f;
        uint8_t elf_class = 2; // 64-bit
        uint8_t data_endianness = 1; // little endian
        uint8_t ei_version = 1;
        uint8_t osabi = 0; // system v
        uint64_t abi_vers_pad = 0;
        uint16_t type = 1; // relocatable file
        uint16_t machine = 0x3e; //amd x86-64
        uint32_t e_version = 1;
        uint64_t e_entry = 0; //entry point for main
        uint64_t e_phoff = 0; //program header table
        uint64_t e_shoff = 0; //section header table
        uint32_t e_flags = 0;
        uint16_t e_ehsize = sizeof(Header); //size of the header
        uint16_t e_phentsize = 0;
        uint16_t e_phnum = 0; // number of entries in the program header table
        uint16_t e_shentsize = 0; // size of the section header table entry
        uint16_t e_shnum = 0; //number of entries in the section header table
        uint16_t e_shstrndx = 0; //index of the section header table entry that contains section names
    };
    static_assert(sizeof(Header) == 64);

    struct SectionHeader {
        uint32_t sh_name = 0; //offset to section name in .shstrtab section
        uint32_t sh_type = 0;
        uint64_t sh_flags = 0;
        uint64_t sh_addr = 0;
        uint64_t sh_offset = 0;
        uint64_t sh_size = 0;
        uint32_t sh_link = 0;
        uint32_t sh_info = 0;
        uint64_t sh_addralign = 0;
        uint64_t sh_entsize = 0;
    };
    static_assert(sizeof(SectionHeader) == 64);

    struct RelocationEntry {
        uint64_t offset;
        uint32_t type;
        uint32_t symbol;
        int64_t addend;
    };

    struct SymbolEntry {
        uint32_t name;
        uint8_t info;
        uint8_t other = 0;
        uint16_t shndx;
        uint64_t value;
        uint64_t size;
    };

    struct RelocationData {
        std::vector<SymbolEntry> symbols;
        std::vector<RelocationEntry> relocs;
        std::vector<uint8_t> strtab;
        std::vector<uint8_t> rodata;

        static RelocationData generate(InstrBufferx64& instrs);
    };
};
