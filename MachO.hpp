
//------------------------------------------------------------------------------
// MachO.hpp
//------------------------------------------------------------------------------

#pragma once

class InstrBufferx64;

#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

namespace macho {
    void write(std::ostream& out, InstrBufferx64& buff);

    struct Header {
        uint32_t magic = 0xfeedfacf;
        uint32_t cputype = 0x1000007;
        uint32_t cpusubtype = 0x3;
        uint32_t filetype = 0x1;
        uint32_t numberofload = 0x04;
        uint32_t sizeofload = 0;
        uint32_t flags = 0x2000;
        uint32_t reserved = 0x0;
    };

    struct LoadSegment64 {
        uint32_t commandtype = 0x19;
        uint32_t commandsize = 72 + 80 + 80;
        char segmentname[16];
        uint64_t address = 0x00;
        uint64_t addresssize = 0x00; // __text + __cstring
        uint64_t fileoffset = 0x00;
        uint64_t filesize = 0x00;
        uint32_t maxvirtualmemprotections = 0x7; //RWE
        uint32_t initialvirtualmemprotections = 0x7; // RWE
        uint32_t numberofsections = 0x2; // __text, __cstring
        uint32_t flag32 = 0x0;
    };

    struct SegmentSection64 {
        char sectionname[16];
        char segmentname[16];
        uint64_t address = 0x00;
        uint64_t addresssize = 0x00;
        uint32_t fileoffset = 0x00;
        uint32_t alignment = 0x4; //copied from clang output 2^4
        uint32_t relocationsfileoff = 0x0; // populate
        uint32_t numberofrelocations = 0x0; 
        uint32_t flagtype = 0x80000400; // unknown, copied from ouput from clang
        uint32_t reserved1 = 0x0;
        uint32_t reserved2 = 0x0;
        uint32_t reserved3 = 0x0;
    };

    struct BuildVersion { //24 bytes
        uint32_t commandtype = 0x32;
        uint32_t commandsize = 24;
        uint32_t platformtype = 0x1;
        uint32_t minosvers = 0x0e0000;
        uint32_t sdkvers = 0x0e0400;
        uint32_t numberoftools = 0x0;
    };

    struct SymbolTable {
        uint32_t commandtype = 0x2;
        uint32_t commandsize = 24; 
        uint32_t symbolsoffset = 0x00; // populate
        uint32_t numberofsymbols = 0x2; 
        uint32_t stringtableoffset = 0x00; // populate
        uint32_t stringtablesize = 0x0; // populate
    };

    struct DynamicSymbolTable { //80 bytes
        uint32_t commandtype = 0x0b;
        uint32_t commandsize = 0x50;

        uint32_t ilocalsym = 0x0;
        uint32_t nlocalsym = 0x0;
        uint32_t iextdefsym = 0x0;
        uint32_t nextdefsym = 0x1; //main
        uint32_t iundefsym = 0x0;
        uint32_t nundefsym = 0x0;
        uint32_t tocoff = 0x0;
        uint32_t ntoc = 0x0;
        uint32_t modtaboff = 0x0;
        uint32_t nmodtab = 0x0;
        uint32_t extrefsymoff = 0x0;
        uint32_t nextrefsyms = 0x0;
        uint32_t indirectsymoff = 0x0;
        uint32_t nindirectsyms = 0x0;
        uint32_t extreloff = 0x0;
        uint32_t nextrel = 0x0;
        uint32_t locreloff = 0x0;
        uint32_t nlocrel = 0x0;
    };

    struct Symbol {
        uint32_t nameoffset = 0x07;
        uint8_t symboltype = 0x01; //bits 1,2,3 = symbol undefined, bit 0 = external symbol
        uint8_t sectionno = 0x00; //0 meaning undefined
        uint16_t datainfo = 0x00;
        uint64_t symboladdress = 0x00;
    };

    struct SymbolData {
        std::map<std::string, uint32_t> _name_to_index;
        std::vector<uint8_t> _data;
        size_t _string_table_size;
        size_t _string_table_offset;

        SymbolTable symbol_table(size_t symbols_offset) const;
        DynamicSymbolTable dynamic_symbol_table() const;

        static SymbolData generate(InstrBufferx64& buff);
    };

    struct CStringData {
        std::map<std::string, uint32_t> _string_to_offset;
        std::vector<uint8_t> _data;

        static CStringData generate(InstrBufferx64& buff);
    };

    struct RelocationData {
        size_t _count;
        std::vector<uint8_t> _data;

        static RelocationData generate(InstrBufferx64& instrs,
            std::vector<uint8_t>& buff,
            const CStringData& cstrings,
            const SymbolData& symbols);
    };


};
