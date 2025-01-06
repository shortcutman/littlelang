
//------------------------------------------------------------------------------
// Compilerx64.tests.cpp
//------------------------------------------------------------------------------

#include "MachO.hpp"

#include "InstrBufferx64.hpp"

#include <gtest/gtest.h>

namespace {
    bool compare_data_to_cstr(uint8_t* start, size_t range, const char* against) {
        return std::equal(
            start,
            start + range,
            reinterpret_cast<const uint8_t*>(against)
        );
    }

    template<typename T>
    bool compare_data_to(uint8_t* start, T against) {
        return std::equal(
            start,
            start + sizeof(T),
            reinterpret_cast<uint8_t*>(&against)
        );
    }
}

TEST(MachOTests, symbol_table_only_main) {
    InstrBufferx64 buffer;
    auto symtable = macho::SymbolData::generate(buffer);

    ASSERT_EQ(symtable._name_to_index.size(), 1);
    ASSERT_FALSE(symtable._data.empty());

    EXPECT_EQ(symtable._data[0], 1); //_main name offset
    EXPECT_EQ(symtable._data[4], 0x0f); //_main symbol type
    EXPECT_EQ(symtable._data[5], 1); //_main section number, __TEXT, __text

    EXPECT_EQ(symtable._data[sizeof(macho::Symbol)], 0x00); //symbol strings null byte buffer
    
    EXPECT_TRUE(compare_data_to_cstr(&symtable._data[sizeof(macho::Symbol) + 1], 6, "_main"));
}

TEST(MachOTests, symbol_table_main_and_extern) {
    InstrBufferx64 buffer;
    buffer._externFuncs.push_back(InstrBufferx64::ExternFunction{
        .symbol = "puts",
        .location = 10
    });

    auto symtable = macho::SymbolData::generate(buffer);

    ASSERT_EQ(symtable._name_to_index.size(), 2);
    ASSERT_FALSE(symtable._data.empty());

    EXPECT_EQ(symtable._data[0], 1); //_main name offset
    EXPECT_EQ(symtable._data[4], 0x0f); //_main symbol type
    EXPECT_EQ(symtable._data[5], 1); //_main section number, __TEXT, __text

    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) + 0], 7); //_puts name offset
    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) + 4], 0x01); //_puts symbol type
    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) + 5], 0); //_puts section number, undefined

    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) * 2], 0x00); //symbol strings null byte buffer
    EXPECT_TRUE(compare_data_to_cstr(&symtable._data[sizeof(macho::Symbol) * 2 + 1], 6, "_main"));
    EXPECT_TRUE(compare_data_to_cstr(&symtable._data[sizeof(macho::Symbol) * 2 + 7], 5, "_puts"));
}

TEST(MachOTests, symbol_table_main_and_extern_duplicated) {
    InstrBufferx64 buffer;
    buffer._externFuncs.push_back(InstrBufferx64::ExternFunction{
        .symbol = "puts",
        .location = 10
    });
    buffer._externFuncs.push_back(InstrBufferx64::ExternFunction{
        .symbol = "puts",
        .location = 15
    });
    buffer._externFuncs.push_back(InstrBufferx64::ExternFunction{
        .symbol = "puts",
        .location = 20
    });

    auto symtable = macho::SymbolData::generate(buffer);

    ASSERT_EQ(symtable._name_to_index.size(), 2);
    ASSERT_FALSE(symtable._data.empty());

    EXPECT_EQ(symtable._data[0], 1); //_main name offset
    EXPECT_EQ(symtable._data[4], 0x0f); //_main symbol type
    EXPECT_EQ(symtable._data[5], 1); //_main section number, __TEXT, __text

    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) + 0], 7); //_puts name offset
    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) + 4], 0x01); //_puts symbol type
    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) + 5], 0); //_puts section number, undefined

    EXPECT_EQ(symtable._data[sizeof(macho::Symbol) * 2], 0x00); //symbol strings null byte buffer
    EXPECT_TRUE(compare_data_to_cstr(&symtable._data[sizeof(macho::Symbol) * 2 + 1], 6, "_main"));
    EXPECT_TRUE(compare_data_to_cstr(&symtable._data[sizeof(macho::Symbol) * 2 + 7], 5, "_puts"));
}

TEST(MachOTests, cstrings_empty) {
    InstrBufferx64 buffer;
    auto cstrings = macho::CStringData::generate(buffer);

    EXPECT_TRUE(cstrings._data.empty());
    EXPECT_TRUE(cstrings._string_to_offset.empty());
}

TEST(MachOTests, cstrings_one) {
    InstrBufferx64 buffer;
    buffer.add_cstring("test", 10);
    auto cstrings = macho::CStringData::generate(buffer);

    EXPECT_EQ(cstrings._data.size(), 5);
    EXPECT_TRUE(compare_data_to_cstr(&cstrings._data[0], 4, "test"));

    EXPECT_EQ(cstrings._string_to_offset.size(), 1);
    EXPECT_EQ(cstrings._string_to_offset["test"], 0);
}

TEST(MachOTests, cstrings_three) {
    InstrBufferx64 buffer;
    buffer.add_cstring("test", 10);
    buffer.add_cstring("another", 5);
    buffer.add_cstring("what", 21);
    auto cstrings = macho::CStringData::generate(buffer);

    EXPECT_EQ(cstrings._data.size(), 18);
    EXPECT_TRUE(compare_data_to_cstr(&cstrings._data[0], 4, "test"));
    EXPECT_TRUE(compare_data_to_cstr(&cstrings._data[5], 7, "another"));
    EXPECT_TRUE(compare_data_to_cstr(&cstrings._data[13], 4, "what"));

    EXPECT_EQ(cstrings._string_to_offset.size(), 3);
    EXPECT_EQ(cstrings._string_to_offset["test"], 0);
    EXPECT_EQ(cstrings._string_to_offset["another"], 5);
    EXPECT_EQ(cstrings._string_to_offset["what"], 13);
}

TEST(MachOTests, relocations_none) {
    InstrBufferx64 buffer;

    std::vector<uint8_t> bufferCopy = buffer.buffer();
    auto relocs = macho::RelocationData::generate(buffer, bufferCopy, macho::CStringData(), macho::SymbolData());
    EXPECT_TRUE(bufferCopy.empty());
    EXPECT_EQ(relocs._count, 0);
    EXPECT_TRUE(relocs._data.empty());
}

TEST(MachOTests, relocations_cstring_one) {
    InstrBufferx64 buffer;
    buffer.add_cstring("test", 2);
    buffer.mov_r64_imm64(InstrBufferx64::Register::RAX, 0); // on heap

    auto cstrings = macho::CStringData::generate(buffer);
    EXPECT_EQ(cstrings._data.size(), 5);
    EXPECT_TRUE(compare_data_to_cstr(&cstrings._data[0], 4, "test"));
    EXPECT_EQ(cstrings._string_to_offset.size(), 1);
    EXPECT_EQ(cstrings._string_to_offset["test"], 0);

    std::vector<uint8_t> bufferCopy = buffer.buffer();
    auto relocs = macho::RelocationData::generate(buffer, bufferCopy, cstrings, macho::SymbolData());
    EXPECT_TRUE(compare_data_to<uint64_t>(&bufferCopy[2], bufferCopy.size()));
    EXPECT_EQ(relocs._count, 1);
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[0], 2));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[4], 0x06000002));
}

TEST(MachOTests, relocations_cstring_three) {
    InstrBufferx64 buffer;
    buffer.add_cstring("test", 2);
    buffer.mov_r64_imm64(InstrBufferx64::Register::RAX, 0);
    buffer.add_cstring("another", 12);
    buffer.mov_r64_imm64(InstrBufferx64::Register::RAX, 0);
    buffer.add_cstring("what", 22);
    buffer.mov_r64_imm64(InstrBufferx64::Register::RAX, 0);

    auto cstrings = macho::CStringData::generate(buffer);
    EXPECT_EQ(cstrings._data.size(), 18);
    EXPECT_EQ(cstrings._string_to_offset.size(), 3);
    EXPECT_EQ(cstrings._string_to_offset["test"], 0);
    EXPECT_EQ(cstrings._string_to_offset["another"], 5);
    EXPECT_EQ(cstrings._string_to_offset["what"], 13);

    std::vector<uint8_t> bufferCopy = buffer.buffer();
    auto relocs = macho::RelocationData::generate(buffer, bufferCopy, cstrings, macho::SymbolData());
    EXPECT_TRUE(compare_data_to<uint64_t>(&bufferCopy[2], bufferCopy.size() + 0));
    EXPECT_EQ(relocs._count, 3);
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[0], 2));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[4], 0x06000002));

    EXPECT_TRUE(compare_data_to<uint64_t>(&bufferCopy[12], bufferCopy.size() + 5));
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[8], 12));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[12], 0x06000002));

    EXPECT_TRUE(compare_data_to<uint64_t>(&bufferCopy[22], bufferCopy.size() + 13));
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[16], 22));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[20], 0x06000002));
}


TEST(MachOTests, relocations_externs_one) {
    InstrBufferx64 buffer;
    buffer._externFuncs.push_back({
        .symbol = "puts",
        .location = 0
    });
    auto symbols = macho::SymbolData::generate(buffer);

    std::vector<uint8_t> bufferCopy = buffer.buffer();
    auto relocs = macho::RelocationData::generate(buffer, bufferCopy, macho::CStringData(), symbols);
    EXPECT_TRUE(bufferCopy.empty());
    EXPECT_EQ(relocs._count, 1);
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[0], 0));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[4], 0x2d000001));
}

TEST(MachOTests, relocations_externs_three) {
    InstrBufferx64 buffer;
    buffer._externFuncs.push_back({
        .symbol = "puts",
        .location = 0
    });
    buffer._externFuncs.push_back({
        .symbol = "printf",
        .location = 10
    });
    buffer._externFuncs.push_back({
        .symbol = "itoa",
        .location = 20
    });
    auto symbols = macho::SymbolData::generate(buffer);

    std::vector<uint8_t> bufferCopy = buffer.buffer();
    auto relocs = macho::RelocationData::generate(buffer, bufferCopy, macho::CStringData(), symbols);
    EXPECT_TRUE(bufferCopy.empty());
    EXPECT_EQ(relocs._count, 3);

    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[0], 0));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[4], 0x2d000001));

    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[8], 10));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[12], 0x2d000002));

    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[16], 20));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[20], 0x2d000003));
}

TEST(MachOTests, relocations_combined_one_each) {
    InstrBufferx64 buffer;
    buffer.add_cstring("test", 2);
    buffer.mov_r64_imm64(InstrBufferx64::Register::RAX, 0); // on heap
    buffer._externFuncs.push_back({
        .symbol = "puts",
        .location = 10
    });
    auto cstrings = macho::CStringData::generate(buffer);
    auto symbols = macho::SymbolData::generate(buffer);

    std::vector<uint8_t> bufferCopy = buffer.buffer();
    auto relocs = macho::RelocationData::generate(buffer, bufferCopy, cstrings, symbols);
    EXPECT_TRUE(compare_data_to<uint64_t>(&bufferCopy[2], bufferCopy.size()));
    EXPECT_EQ(relocs._count, 2);
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[0], 2));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[4], 0x06000002));
    EXPECT_TRUE(compare_data_to<int32_t>(&relocs._data[8], 10));
    EXPECT_TRUE(compare_data_to<uint32_t>(&relocs._data[12], 0x2d000001));
}