
//------------------------------------------------------------------------------
// InstrBuffer.hpp
//------------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

class InstrBufferx64 {
public:
    enum class Register {
        RAX = 0,
        RCX = 1,
        RDX = 2,
        RBX = 3,
        RSP = 4,
        RBP = 5,
        RSI = 6,
        RDI = 7
    };

    struct JmpUpdate {
        InstrBufferx64* owner;
        size_t location;
    };

    struct CString {
        std::string string;
        size_t location;
        auto operator<=>(const CString&) const = default;
    };
    std::vector<std::unique_ptr<CString>> _cstrings;

    struct ExternFunction {
        std::string symbol;
        size_t location;
        auto operator<=>(const ExternFunction&) const = default;
    };
    std::vector<ExternFunction> _externFuncs;

private:
    std::vector<std::uint8_t> _buffer;
    std::vector<std::unique_ptr<JmpUpdate>> _updates;

public:
    InstrBufferx64() {}

    void execute(std::size_t entrypoint = 0);
    const std::vector<std::uint8_t>& buffer() const;
    std::vector<std::uint8_t>& buffer();

    uint64_t add_cstring(const std::string& str, size_t location);

    void mov_r64_r64(Register dest, Register src);
    void mov_r64_imm64(Register dest, std::uint64_t input);
    void lea_r64_riprel32(Register dest, std::int32_t input);
    void mov_stack_imm64(std::int8_t adjust, std::uint64_t value);
    void mov_stack_r64(std::int8_t adjust, Register src);
    void mov_r64_stack(Register dest, std::int8_t adjust);

    void add_r64_imm32(Register dest, std::int32_t value);
    void add_r64_r64(Register dest, Register src);

    void sub(Register dest, std::int32_t value);

    void cqo_idiv_r64(Register src);

    void cmp(Register a, Register b);
    void jmp_not_equal(int32_t offset);
    void jmp_greater_or_equal(int32_t offset);

    JmpUpdate* jmp_with_update();
    void update_jmp(JmpUpdate* update, int32_t offset);

    void append_buffer(InstrBufferx64& buffer);
    
    void call_r64(Register dest);
    void call_rel32(int32_t op);
    void ret();

    void push(Register src);
    void pop(Register dest);

private:
    void push_rexw();
    void push_modrm(uint8_t mod, uint8_t regop, uint8_t rm);
    void push_modrm(uint8_t mod, uint8_t regop, Register rm);
    void push_modrm(uint8_t mod, Register regop, Register rm);
    void push_byte(uint8_t byte);
    void push_dword(uint32_t dword);
    void push_qword(uint64_t qword);
};
