
#include "Compilerx64.hpp"
#include "Elf.hpp"
#include "InstrBufferx64.hpp"
#include "MachO.hpp"
#include "Parser.hpp"
#include "TranslationUnit.hpp"

#include "vendor/cli11/CLI11.hpp"

#include <iostream>
#include <fstream>
#include <string>

namespace {
    const std::string fizzbuzz_program = R"(
    int64 counter;
    counter = 1;
    while (counter < 100) {
        int64 mod3;
        mod3 = counter % 3;

        int64 mod5;
        mod5 = counter % 5;
        if (mod3 == 0) {
            if (mod5 == 0) {
                printf("FizzBuzz");
            } else {
                printf("Fizz");
            }
        } else if (mod5 == 0) {
            printf("Buzz");
        } else {
            printf("%i", counter);
        }
        counter = counter + 1;
        puts("");
    }
    )";
}

void fizzbuzz_jit() {
    Parser p;
    p.parse_block(fizzbuzz_program);
    InstrBufferx64 i;
    auto compiler = Compiler_x64(p.block.get(), &i, Compiler_x64::Mode::JIT);
    compiler.compile_function();
    i.execute(0);
}

void fizzbuzz_bin() {
    Parser p;
    p.parse_block(fizzbuzz_program);
    InstrBufferx64 i;
    auto compiler = Compiler_x64(p.block.get(), &i, Compiler_x64::Mode::ObjectFile);
    compiler.compile_function();

    std::fstream f("./out.o", f.binary | f.out);
    if (!f.is_open()) {
        throw std::runtime_error("couldn't open");
    }

#ifdef __APPLE__
    macho::write(f, i);
    f.close();
#elif __linux__
    elf::write(f, i);
    f.close();
#endif
}


int main(int argc, char** argv) {
    enum class ObjectFileType { Macho, ELF };

    std::map<std::string, Compiler_x64::Mode> compilerModeMap{{"jit", Compiler_x64::Mode::JIT}, {"object", Compiler_x64::Mode::ObjectFile}};
    std::map<std::string, ObjectFileType> objectFileTypeMap{{"macho", ObjectFileType::Macho}, {"elf", ObjectFileType::ELF}};

    std::string file;
    Compiler_x64::Mode mode{Compiler_x64::Mode::JIT};
    ObjectFileType objectFileType;
    std::string outputFile;
    bool linkExe = false;

    CLI::App app{"Littlelang is a simple programming language that is compiled to machine code for either executables or run in-memory.", "littlelang"};

    app.add_option("file", file, "An input file.")->required();
    app.add_option("-m,--mode", mode, "Compile and run mode.")->transform(CLI::CheckedTransformer(compilerModeMap, CLI::ignore_case));
    auto optObj = app.add_option("-t,--object-type", objectFileType, "Object file type to ouput.")->transform(CLI::CheckedTransformer(objectFileTypeMap, CLI::ignore_case));
    app.add_option("-o,--output", outputFile, "Output file.")->needs(optObj);
    app.add_flag("-L, --link", linkExe, "Link output to an exe utilising the system linker.")->needs(optObj);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    std::ifstream program_file(file);
    if (!program_file.is_open()) {
        std::cout << "Unable to open file: " << file << std::endl;
        return 1;
    }

    std::stringstream program_text_stream;
    program_text_stream << program_file.rdbuf();
    auto program_text = program_text_stream.str();

    InstrBufferx64 instrbuff;

    if (mode == Compiler_x64::Mode::JIT) {
        struct FunctionSymbol {
            std::string name;
            std::size_t offset;
        };
        std::vector<FunctionSymbol> symbols;
    
        auto sv = std::string_view{program_text};
        auto tu = ll::TranslationUnit::parse_translation_unit(sv);
        for (auto& func : tu->functions) {
            symbols.push_back(FunctionSymbol{
                .name = func->name,
                .offset = instrbuff.buffer().size()
            });
            auto compiler = Compiler_x64(func->block.get(), &instrbuff, mode);
            compiler.compile_function();
        }
    
        auto entryPoint = std::find_if(symbols.begin(), symbols.end(), [] (auto& v) {
            return v.name == "main";
        });

        instrbuff.execute(entryPoint->offset);
    } else if (mode == Compiler_x64::Mode::ObjectFile) {
        Parser parser;
        parser.parse_block(program_text);
        auto compiler = Compiler_x64(parser.block.get(), &instrbuff, mode);
        compiler.compile_function();

        std::fstream objectFile(outputFile, std::fstream::binary | std::fstream::out);
        if (!objectFile.is_open()) {
            std::cout << "Couldn't open file path for writing out object file: " << outputFile << std::endl;
            return 1;
        }

        switch (objectFileType) {
            case ObjectFileType::Macho:
            {
                macho::write(objectFile, instrbuff);
                objectFile.close();

                if (linkExe) {
                    std::stringstream ss;
                    ss << "ld -ld_classic -arch x86_64 -o ll_bin -syslibroot /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk -lSystem ";
                    ss << outputFile;

                    auto linkres = system(ss.str().c_str());

                    if (linkres != 0) {
                        std::cout << "Linker exited with error: " << linkres << std::endl;
                    }
                }

                break;
            }

            case ObjectFileType::ELF:
            {
                elf::write(objectFile, instrbuff);
                objectFile.close();

                if (linkExe) {
                    std::stringstream ss;
                    ss << "ld -o ll_bin -dynamic-linker /lib64/ld-linux-x86-64.so.2 -lc /usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crti.o /usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/Scrt1.o /usr/lib/gcc/x86_64-linux-gnu/13/crtbeginS.o /usr/lib/gcc/x86_64-linux-gnu/13/crtendS.o /usr/lib/gcc/x86_64-linux-gnu/13/../../../x86_64-linux-gnu/crtn.o ";
                    ss << outputFile;

                    auto linkres = system(ss.str().c_str());

                    if (linkres != 0) {
                        std::cout << "Linker exited with error: " << linkres << std::endl;
                    }
                }

                break;
            }

            default:
            {
                std::cout << "Unknown object file type." << std::endl;
                break;
            }
        }
    }

    return 0;
}
