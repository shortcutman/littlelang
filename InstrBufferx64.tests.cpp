
#include "InstrBufferx64.hpp"

#include <gtest/gtest.h>

TEST(InstrBufferx64, call_rax) {
    InstrBufferx64 b;
    b.call_r64(InstrBufferx64::Register::RAX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0xff, 0xd0}));
}

TEST(InstrBufferx64, call_rdi) {
    InstrBufferx64 b;
    b.call_r64(InstrBufferx64::Register::RDI);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0xff, 0xd7}));
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
    b.mov_r64_imm64(InstrBufferx64::Register::RAX, 0x1122334455667788);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11}));
}

TEST(InstrBufferx64, mov_stack_imm64) {
    InstrBufferx64 b;
    b.mov_stack_imm64(-8, 0x1122334455667788);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x48, 0xb8, 0x88, 0x77, 0x66, 0x55, 0x44, 0x33, 0x22, 0x11, // mov rax, imm64
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 0x8], rax
        }));
}

TEST(InstrBufferx64, mov_stack_r64) {
    InstrBufferx64 b;
    b.mov_stack_r64(-8, InstrBufferx64::Register::RAX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x48, 0x89, 0x45, 0xf8 //mov [rbp - 8], rax
        }));
}

TEST(InstrBufferx64, mov_r64_stack) {
    InstrBufferx64 b;
    b.mov_r64_stack(InstrBufferx64::Register::RSI, -8);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x48, 0x8b, 0x75, 0xf8
        }));
}

TEST(InstrBufferx64, push_rbp) {
    InstrBufferx64 b;
    b.push(InstrBufferx64::Register::RBP);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0xff, 0xf5}));
}

TEST(InstrBufferx64, pop_rbp) {
    InstrBufferx64 b;
    b.pop(InstrBufferx64::Register::RBP);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x5d}));
}

TEST(InstrBufferx64, mov_rbp_rsp) {
    InstrBufferx64 b;
    b.mov_r64_r64(InstrBufferx64::Register::RBP, InstrBufferx64::Register::RSP);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0x89, 0xe5}));
}

TEST(InstrBufferx64, sub_rsp_0x10) {
    InstrBufferx64 b;
    b.sub(InstrBufferx64::Register::RSP, 0x10);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0x81, 0xec, 0x10, 0x00, 0x00, 0x00}));
}

TEST(InstrBufferx64, add_r64_imm32) {
    InstrBufferx64 b;
    b.add_r64_imm32(InstrBufferx64::Register::RSP, 0x10);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0x81, 0xc4, 0x10, 0x00, 0x00, 0x00}));
}

TEST(InstrBufferx64, add_r64_r64) {
    InstrBufferx64 b;
    b.add_r64_r64(InstrBufferx64::Register::RAX, InstrBufferx64::Register::RCX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({0x48, 0x03, 0xc1}));
}
