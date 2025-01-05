
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