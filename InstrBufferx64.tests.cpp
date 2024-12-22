
#include "InstrBufferx64.hpp"

#include <gtest/gtest.h>

TEST(InstrBufferx64, Call) {
    InstrBufferx64 b;
    b.call_r64(InstrBufferx64::Register::RAX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0xff, 0xd0}));
}

TEST(InstrBufferx64, Return) {
    InstrBufferx64 b;
    b.ret();
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0xc3}));
}

TEST(InstrBufferx64, Mov_r64_imm64) {
    InstrBufferx64 b;
    b.push_mov_r64_imm64(InstrBufferx64::RAX, 0x1122334455667788);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11}));
}

TEST(InstrBufferx64, push_rbp) {
    InstrBufferx64 b;
    b.push(InstrBufferx64::RBP);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0xff, 0xf5}));
}

TEST(InstrBufferx64, mov_rbp_rsp) {
    InstrBufferx64 b;
    b.mov(InstrBufferx64::RBP, InstrBufferx64::RSP);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0x89, 0xe5}));
}

TEST(InstrBufferx64, sub_rsp_0x10) {
    InstrBufferx64 b;
    b.sub(InstrBufferx64::RSP, 0x10);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0x81, 0xec, 0x10, 0x00, 0x00, 0x00}));
}
