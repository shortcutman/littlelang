
//------------------------------------------------------------------------------
// InstrBuffer.cpp
//------------------------------------------------------------------------------

#include "InstrBufferx64.hpp"

#include <sys/mman.h>

void InstrBufferx64::execute() {
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

void InstrBufferx64::push_mov_r64_imm64(Register dest, std::uint64_t input) {
    push_rexw();
    push_byte(0xb8 + (static_cast<int>(dest) & 0x07));
    push_qword(input);
}

void InstrBufferx64::call_r64(Register dest) {
    push_byte(0xff);
    push_modrm(3, 2, static_cast<int>(dest) & 0x03);
}

void InstrBufferx64::ret() {
    push_byte(0xc3);
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

void InstrBufferx64::push_byte(uint8_t byte) {
    _buffer.push_back(byte);
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
