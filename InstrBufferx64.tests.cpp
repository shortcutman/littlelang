
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

TEST(InstrBufferx64, idiv_rax_throw) {
    InstrBufferx64 b;
    EXPECT_ANY_THROW(b.cqo_idiv_r64(InstrBufferx64::Register::RAX));
}

TEST(InstrBufferx64, idiv_rdx_throw) {
    InstrBufferx64 b;
    EXPECT_ANY_THROW(b.cqo_idiv_r64(InstrBufferx64::Register::RDX));
}

TEST(InstrBufferx64, idiv_rcx) {
    InstrBufferx64 b;
    b.cqo_idiv_r64(InstrBufferx64::Register::RCX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x48, 0x99, //cqo
            0x48, 0xf7, 0xf9 //idiv rax
        }));
}

TEST(InstrBufferx64, cmp_rax_rcx) {
    InstrBufferx64 b;
    b.cmp(InstrBufferx64::Register::RAX, InstrBufferx64::Register::RCX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x48, 0x3b, 0xc1
        }));
}

TEST(InstrBufferx64, cmp_rdx_rbx) {
    InstrBufferx64 b;
    b.cmp(InstrBufferx64::Register::RDX, InstrBufferx64::Register::RBX);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x48, 0x3b, 0xd3
        }));
}

TEST(InstrBufferx64, jmp_not_equal) {
    InstrBufferx64 b;
    b.jmp_not_equal(0x20);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0x0f, 0x85, 0x20, 0x00, 0x00, 0x00
        }));
}

TEST(InstrBufferx64, jmp_with_update) {
    InstrBufferx64 b;
    auto update = b.jmp_with_update();
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0xe9, 0xef, 0xbe, 0xad, 0xde
        }));
    EXPECT_EQ(update->location, 1);
}

TEST(InstrBufferx64, jmp_with_update_and_do_update) {
    InstrBufferx64 b;
    auto update = b.jmp_with_update();
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0xe9, 0xef, 0xbe, 0xad, 0xde
        }));
    EXPECT_EQ(update->location, 1);

    b.update_jmp(update, 0xcafebabe);
    EXPECT_EQ(
        b.buffer(),
        std::vector<uint8_t>({
            0xe9, 0xbe, 0xba, 0xfe, 0xca
        }));
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
