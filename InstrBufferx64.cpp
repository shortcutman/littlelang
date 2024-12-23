
//------------------------------------------------------------------------------
// InstrBuffer.cpp
//------------------------------------------------------------------------------

#include "InstrBufferx64.hpp"

#include <sys/mman.h>

void InstrBufferx64::execute() {
    if (_buffer.empty()) {
        return;
    }

    void* exememory = mmap(NULL,
        _buffer.size(),
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANON | MAP_PRIVATE | MAP_JIT,
        -1,
        0);
    memset(exememory, 0, _buffer.size());
    memcpy(exememory, &_buffer[0], _buffer.size());
    reinterpret_cast<void(*)(void)>(exememory)();
}

std::vector<uint8_t> InstrBufferx64::buffer() const {
    return _buffer;
}

void InstrBufferx64::mov_r64_imm64(Register dest, std::uint64_t input) {
    push_rexw();
    push_byte(0xb8 + (static_cast<int>(dest) & 0x07));
    push_qword(input);
}

void InstrBufferx64::mov_stack_imm64(std::int8_t adjust, std::uint64_t value) {
    push_rexw();
    push_byte(0xb8 + (static_cast<uint8_t>(Register::RAX) & 0x07));
    push_qword(value);

    push_rexw();
    push_byte(0x89);
    push_modrm(1, Register::RAX, Register::RBP);
    push_byte(*reinterpret_cast<uint8_t*>(&adjust));
}

void InstrBufferx64::mov_stack_r64(std::int8_t adjust, Register src) {
    push_rexw();
    push_byte(0x89);
    push_modrm(1, src, Register::RBP);
    push_byte(*reinterpret_cast<uint8_t*>(&adjust));
}

void InstrBufferx64::mov_r64_stack(Register dest, std::int8_t adjust) {
    push_rexw();
    push_byte(0x8b);
    push_modrm(1, dest, Register::RBP);
    push_byte(*reinterpret_cast<uint8_t*>(&adjust));
}

void InstrBufferx64::call_r64(Register dest) {
    push_byte(0xff);
    push_modrm(3, 2, static_cast<int>(dest) & 0x07);
}

void InstrBufferx64::add_r64_imm32(Register dest, std::int32_t value) {
    push_rexw();
    push_byte(0x81);
    push_modrm(3, 0, dest);
    push_dword(*reinterpret_cast<uint32_t*>(&value));
}

void InstrBufferx64::ret() {
    push_byte(0xc3);
}

void InstrBufferx64::push(Register src) {
    push_byte(0xff);
    push_modrm(3, 6, static_cast<int>(src) & 0x07);
}

void InstrBufferx64::pop(Register dest) {
    push_byte(0x58 + (static_cast<int>(dest) & 0x07));
}

void InstrBufferx64::mov_r64_r64(Register dest, Register src) {
    push_rexw();
    push_byte(0x89);
    push_modrm(3, /* regop src */ src, /* rm dest */ dest);
}

void InstrBufferx64::sub(Register dest, std::int32_t value) {
    push_rexw();
    push_byte(0x81);
    push_modrm(3, 5, dest);
    push_dword(*reinterpret_cast<uint32_t*>(&value));
}

void InstrBufferx64::push_rexw() {
    uint8_t byte = 0b01001000;
    push_byte(byte);
}

void InstrBufferx64::push_modrm(uint8_t mod, uint8_t regop, uint8_t rm) {
    uint8_t byte = 0;
    byte |= (mod & 0x03) << 6;
    byte |= (regop & 0x07) << 3;
    byte |= (rm & 0x07);
    push_byte(byte);
}

void InstrBufferx64::push_modrm(uint8_t mod, uint8_t regop, Register rm) {
    push_modrm(mod, regop, static_cast<uint8_t>(rm));
}

void InstrBufferx64::push_modrm(uint8_t mod, Register regop, Register rm) {
    push_modrm(mod, static_cast<uint8_t>(regop), static_cast<uint8_t>(rm));
}

void InstrBufferx64::push_byte(uint8_t byte) {
    _buffer.push_back(byte);
}

void InstrBufferx64::push_dword(uint32_t dword) {
    push_byte(dword & 0xff);
    push_byte((dword >> 8) & 0xff);
    push_byte((dword >> 16) & 0xff);
    push_byte((dword >> 24) & 0xff);
}

void InstrBufferx64::push_qword(uint64_t qword) {
    push_byte(qword & 0xff);
    push_byte((qword >> 8) & 0xff);
    push_byte((qword >> 16) & 0xff);
    push_byte((qword >> 24) & 0xff);
    push_byte((qword >> 32) & 0xff);
    push_byte((qword >> 40) & 0xff);
    push_byte((qword >> 48) & 0xff);
    push_byte((qword >> 56) & 0xff);
}
