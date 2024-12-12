
#include <iostream>
#include <sys/mman.h>
#include <string.h>
#include <vector>
#include <stdio.h>
#include <dlfcn.h>

void anotherfunction() {
    const char* ant = "hello";
    puts(ant);
}

void func2() {
    const char* another = "func2";

    asm (R"(
        sub rsp, 16
        mov rdi, qword ptr [rbp - 8]
        call _puts
    )");

    asm (R"(
        mov rdi, qword ptr [rbp - 8]
        call _puts
        add rsp, 16
    )");
}

void func3() {
    const char* another = "func3";

    asm (R"(
        call _puts
    )"
    :
    : "D" (another));

    asm (R"(
        mov rdi, qword ptr [rbp - 8]
        call _puts
    )");
}

void func4() {
    const char* another = "func4";
    puts(another);

    printf("puts: %p\n", puts);
    void* dlHandle = dlopen(0, RTLD_NOW);
    void* isitputs = dlsym(dlHandle, "puts");
    printf("puts: %p\n", isitputs);

    asm (R"(
        mov rdi, qword ptr [rbp - 8]
        call rax
    )"
    :
    : "a" (isitputs));
}

typedef void(*funky)(void);

void func5() {
    const char* another = "func5";
    void* dlHandle = dlopen(0, RTLD_NOW);
    void* putsaddr = dlsym(dlHandle, "puts");

    std::vector<uint8_t> a {
        0x48, 0xb8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rax, imm64
        0x48, 0xbf, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, //mov rdi, imm64
        0xff, 0xd0, //call rax
        0xc3 //ret
    };

    memcpy(&a[2], &putsaddr, 8);
    memcpy(&a[12], &another, 8);

    std::vector<uint8_t> b;
    b.insert(b.end(), {0x48, 0xb8});

    uint8_t* bytes = reinterpret_cast<uint8_t*>(&putsaddr);
    b.insert(b.end(), bytes, bytes + 8);

    void* exememory = mmap(NULL,
        1024,
        PROT_READ | PROT_WRITE | PROT_EXEC,
        MAP_ANON | MAP_PRIVATE | MAP_JIT,
        -1,
        0);
    memset(exememory, 0, 1024);
    memcpy(exememory, &a[0], a.size());

    ((funky)exememory)();
}

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

class InstrBuffer {
private:
    std::vector<std::uint8_t> _buffer;

public:
    InstrBuffer() {}

    void push_mov_r64_imm64(Register dest, std::uint64_t input) {
        push_rexw();
        push_byte(0xb8 + (static_cast<int>(dest) & 0x07));
        push_qword(input);
    }

    void call_r64(Register dest) {
        push_byte(0xff);
        push_modrm(3, 2, static_cast<int>(dest) & 0x03);
    }

    void ret() {
        push_byte(0xc3);
    }

private:
    void push_rexw() {
        uint8_t byte = 0b01001000;
        push_byte(byte);
    }

    void push_modrm(uint8_t mod, uint8_t regop, uint8_t rm) {
        uint8_t byte = 0;
        byte |= (mod & 0x03) << 6;
        byte |= (regop & 0x07) << 3;
        byte |= (rm & 0x07);
        push_byte(byte);
    }

    void push_byte(uint8_t byte) {
        _buffer.push_back(byte);
    }

    void push_qword(uint64_t qword) {
        push_byte(qword & 0xff);
        push_byte((qword >> 8) & 0xff);
        push_byte((qword >> 16) & 0xff);
        push_byte((qword >> 24) & 0xff);
        push_byte((qword >> 32) & 0xff);
        push_byte((qword >> 40) & 0xff);
        push_byte((qword >> 48) & 0xff);
        push_byte((qword >> 56) & 0xff);
    }
};

void func6() {
    const char* another = "func5";
    void* dlHandle = dlopen(0, RTLD_NOW);
    void* putsaddr = dlsym(dlHandle, "puts");

    InstrBuffer b;
    b.push_mov_r64_imm64(Register::RAX, reinterpret_cast<uint64_t>(putsaddr));
    b.push_mov_r64_imm64(Register::RDI, reinterpret_cast<uint64_t>(another));
    b.call_r64(Register::RAX);
    b.ret();
}

int main() {
    anotherfunction();
    func2();
    func3();
    func4();
    func5();
    func6();
    return 0;
}
