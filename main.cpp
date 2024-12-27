
#include "Compilerx64.hpp"
#include "InstrBufferx64.hpp"
#include "Parser.hpp"

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

void func6() {
    const char* another = "func6";
    void* dlHandle = dlopen(0, RTLD_NOW);
    void* putsaddr = dlsym(dlHandle, "puts");

    InstrBufferx64 b;
    b.mov_r64_imm64(InstrBufferx64::Register::RAX, reinterpret_cast<uint64_t>(putsaddr));
    b.mov_r64_imm64(InstrBufferx64::Register::RDI, reinterpret_cast<uint64_t>(another));
    b.call_r64(InstrBufferx64::Register::RAX);
    b.ret();
    b.execute();
}

void func7() {
    std::string example = R"(puts("func7");)";

    Parser p;
    auto call = p.parse_function_call(example);
    InstrBufferx64 i;
    auto compiler = Compiler_x64(&p.block, &i);
    compiler.compile_function_call(*call);
    i.ret();
    
    i.execute();
}

void func8() {
    std::string eg = R"(printf("test %i", 123);)";

    Parser p;
    auto call = p.parse_function_call(eg);
    InstrBufferx64 i;
    auto compiler = Compiler_x64(&p.block, &i);
    compiler.compile_function_call(*call);
    i.ret();
    i.execute();
    printf("\n");
}

void func9() {
    std::string eg = R"(
    int64 another;
    another = 1;
    while (another < 5) {
        printf("%i ", another);
        another = another + 1;
    }
    printf("another %i", another);
    )";

    Parser p;
    p.parse_block(eg);
    InstrBufferx64 i;
    auto compiler = Compiler_x64(&p.block, &i);
    compiler.compile_function();
    i.execute();
}

int main() {
    anotherfunction();
    func2();
    func3();
    func4();
    func5();
    func6();
    func7();
    func8();
    func9();
    return 0;
}
