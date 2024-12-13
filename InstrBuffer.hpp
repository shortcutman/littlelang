
//------------------------------------------------------------------------------
// InstrBuffer.hpp
//------------------------------------------------------------------------------

#include <cstdint>
#include <vector>

class InstrBuffer {
private:
    std::vector<std::uint8_t> _buffer;

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

public:
    InstrBuffer() {}

    void execute();

    void push_mov_r64_imm64(Register dest, std::uint64_t input);
    void call_r64(Register dest);
    void ret();

private:
    void push_rexw();
    void push_modrm(uint8_t mod, uint8_t regop, uint8_t rm);
    void push_byte(uint8_t byte);
    void push_qword(uint64_t qword);
};
