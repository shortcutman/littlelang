
//------------------------------------------------------------------------------
// MachO.hpp
//------------------------------------------------------------------------------

#pragma once

class InstrBufferx64;

#include <map>
#include <ostream>
#include <vector>

namespace macho {
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

        static SymbolData generate(InstrBufferx64& buff);
    };

    struct CStringData {
        std::map<std::string, uint32_t> _string_to_offset;
        std::vector<uint8_t> _data;

        static CStringData generate(InstrBufferx64& buff);
    };
};
