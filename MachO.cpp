
//------------------------------------------------------------------------------
// MachO.cpp
//------------------------------------------------------------------------------

#include "MachO.hpp"

void macho::write(std::ostream& out, std::vector<uint8_t>& buff) {
    struct { //32 bytes
        uint32_t magic = 0xfeedfacf;
        uint32_t cputype = 0x1000007;
        uint32_t cpusubtype = 0x3;
        uint32_t filetype = 0x1;
        uint32_t numberofload = 0x04;
        uint32_t sizeofload = 0;
        uint32_t flags = 0x2000;
        uint32_t reserved = 0x0;
    } machoheader;

    struct { //72 bytes
        uint32_t commandtype = 0x19;
        uint32_t commandsize = 72 + 80 + 80;
        char segmentname[16] = {'_', '_', 'T', 'E', 'X', 'T', 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        uint64_t address = 0x00;
        uint64_t addresssize = 0x00; // __text + __cstring
        uint64_t fileoffset = 0x00;
        uint64_t filesize = 0x00;
        uint32_t maxvirtualmemprotections = 0x7; //RWE
        uint32_t initialvirtualmemprotections = 0x7; // RWE
        uint32_t numberofsections = 0x2; // __text, __cstring
        uint32_t flag32 = 0x0;
    } load_segment_64;

    struct  { //80 bytes
        char sectionname[16] = {'_', '_', 't', 'e', 'x', 't', 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        char segmentname[16] = {'_', '_', 'T', 'E', 'X', 'T', 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        uint64_t address = 0x00;
        uint64_t addresssize = 0x00;
        uint32_t fileoffset = 0x00;
        uint32_t alignment = 0x4; //copied from clang output 2^4
        uint32_t relocationsfileoff = 0x0; // populate
        uint32_t numberofrelocations = 0x2; 
        uint32_t flagtype = 0x80000400; // unknown, copied from ouput from clang
        uint32_t reserved1 = 0x0;
        uint32_t reserved2 = 0x0;
        uint32_t reserved3 = 0x0;
    } segement_section_text;

    struct  { //80 bytes
        char sectionname[16] = {'_', '_', 'c', 's', 't', 'r', 'i', 'n','g',0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        char segmentname[16] = {'_', '_', 'T', 'E', 'X', 'T', 0x00, 0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
        uint64_t address = 0x00; //populate
        uint64_t addresssize = 0x00; //populate
        uint32_t fileoffset = 0x00; //populate
        uint32_t alignment = 0x0; //copied from clang output 2^0
        uint32_t relocationsfileoff = 0x0;
        uint32_t numberofrelocations = 0x0; 
        uint32_t flagtype = 0x2; // unknown, copied from ouput from clang
        uint32_t reserved1 = 0x0;
        uint32_t reserved2 = 0x0;
        uint32_t reserved3 = 0x0;
    } segement_section_cstring;

    struct  { //24 bytes
        uint32_t commandtype = 0x32;
        uint32_t commandsize = 24;
        uint32_t platformtype = 0x1;
        uint32_t minosvers = 0x0e0000;
        uint32_t sdkvers = 0x0e0400;
        uint32_t numberoftools = 0x0;
    } build_version;

    struct  { //24 bytes
        uint32_t commandtype = 0x2;
        uint32_t commandsize = 24; 
        uint32_t symbolsoffset = 0x00; // populate
        uint32_t numberofsymbols = 0x2; 
        uint32_t stringtableoffset = 0x00; // populate
        uint32_t stringtablesize = 0x0; // populate
    } symtab;

    struct  { //80 bytes
        uint32_t commandtype = 0x0b;
        uint32_t commandsize = 0x50;

        uint32_t ilocalsym = 0x0;
        uint32_t nlocalsym = 0x0;
        uint32_t iextdefsym = 0x0;
        uint32_t nextdefsym = 0x1; //main
        uint32_t iundefsym = 0x1;
        uint32_t nundefsym = 0x1; //printf
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
    } dsymtab;

    machoheader.sizeofload = 72 + 80 + 80 + 24 + 24 + 80;
    out.write(reinterpret_cast<std::ostream::char_type*>(&machoheader), sizeof(machoheader));
    
    load_segment_64.addresssize = buff.size() + strlen("Hello world!") + 1; //+1 for null
    load_segment_64.fileoffset = 32 + machoheader.sizeofload;
    load_segment_64.filesize = buff.size() + strlen("Hello world!") + 1; //+1 for null
    out.write(reinterpret_cast<std::ostream::char_type*>(&load_segment_64), sizeof(load_segment_64));

    segement_section_text.addresssize = buff.size();
    segement_section_text.fileoffset = load_segment_64.fileoffset;
    segement_section_text.relocationsfileoff = load_segment_64.fileoffset + load_segment_64.filesize;
    out.write(reinterpret_cast<std::ostream::char_type*>(&segement_section_text), sizeof(segement_section_text));

    segement_section_cstring.address = segement_section_text.addresssize;
    segement_section_cstring.addresssize = strlen("Hello world!") + 1;
    segement_section_cstring.fileoffset = load_segment_64.fileoffset + segement_section_text.addresssize;
    out.write(reinterpret_cast<std::ostream::char_type*>(&segement_section_cstring), sizeof(segement_section_cstring));

    out.write(reinterpret_cast<std::ostream::char_type*>(&build_version), sizeof(build_version));

    symtab.symbolsoffset = load_segment_64.fileoffset + segement_section_text.addresssize + segement_section_cstring.addresssize + 16;
    symtab.stringtableoffset = symtab.symbolsoffset + 16 + 16; //
    symtab.stringtablesize = 13; //
    out.write(reinterpret_cast<std::ostream::char_type*>(&symtab), sizeof(symtab));

    out.write(reinterpret_cast<std::ostream::char_type*>(&dsymtab), sizeof(dsymtab));

    //instructions in __text section
    auto modbuff = buff;
    //string addr
    modbuff[7] = 0x16;
    modbuff[8] = 0x00;
    modbuff[9] = 0x00;
    modbuff[10] = 0x00;
    modbuff[11] = 0x00;
    modbuff[12] = 0x00;
    modbuff[13] = 0x00;
    modbuff[14] = 0x00;

    //call addr, 0x1d outside of commands, 
    modbuff[16] = 0x00;
    modbuff[17] = 0x00;
    modbuff[18] = 0x00;
    modbuff[19] = 0x00;
    out.write(reinterpret_cast<std::ostream::char_type*>(&modbuff[0]), modbuff.size());

    std::string cstrings = "Hello World!";
    out.write(reinterpret_cast<std::ostream::char_type*>(const_cast<char*>(cstrings.c_str())), cstrings.size() + 1);

    struct { //8 bytes
        int32_t address = 0x07;
        uint32_t sym_flags = 0x06000002;
    } reloc_info_string;
    out.write(reinterpret_cast<std::ostream::char_type*>(&reloc_info_string), sizeof(reloc_info_string));
    struct { //8 bytes
        int32_t address = 0x10;
        uint32_t sym_flags = 0x2d000001;
    } reloc_info_puts;
    out.write(reinterpret_cast<std::ostream::char_type*>(&reloc_info_puts), sizeof(reloc_info_puts));

    struct {
        uint32_t nameoffset = 0x01;
        uint8_t symboltype = 0x0f; //bits 1,2,3 = symbol defined in section number, bit 0 = external symbol
        uint8_t sectionno = 0x01;
        uint16_t datainfo = 0x00;
        uint64_t symboladdress = 0x00;
    } symbol64_main;
    out.write(reinterpret_cast<std::ostream::char_type*>(&symbol64_main), sizeof(symbol64_main));

    struct {
        uint32_t nameoffset = 0x07;
        uint8_t symboltype = 0x01; //bits 1,2,3 = symbol undefined, bit 0 = external symbol
        uint8_t sectionno = 0x00; //0 meaning undefined
        uint16_t datainfo = 0x00;
        uint64_t symboladdress = 0x00;
    } symbol64_puts;
    out.write(reinterpret_cast<std::ostream::char_type*>(&symbol64_puts), sizeof(symbol64_puts));

    out.put(0x00);
    std::string maincstr = "_main";
    out.write(reinterpret_cast<std::ostream::char_type*>(const_cast<char*>(maincstr.c_str())), maincstr.size() + 1);

    std::string putscstring = "_puts";
    out.write(reinterpret_cast<std::ostream::char_type*>(const_cast<char*>(putscstring.c_str())), putscstring.size() + 1);
}