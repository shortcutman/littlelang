# LittleLang

This is a little language that is currently compiled into x86_64 op code and executed from memory. Inspired to do it as a way to learn x86_64 machine code with plenty of progamming around it.

It has int64 variables, string constants and can do some basic logic: if/elseif/else blocks, and while loops. My target was to write and run FizzBuzz so only the operations I needed for that have been implemented.

Some future learning ideas from this:
* ARM64 compilation.
* Mach-o binary creation.
* ELF binary creation.

## References

https://kuterdinel.com/writing-a-very-simple-jit-compiler-in-about-1000-lines-of-c.html

I stumbled on this link on Twitter/X which was the original inspiration. It provided main shaping guidance and the kick start on x86_64 assembly.

https://www.ibiblio.org/gferg/ldp/GCC-Inline-Assembly-HOWTO.html

To start with getting the interaction between raw op code bytes and the C++ I was writing, I started with inline assembly, above was helpful and the context to all the junk in main.cpp.

https://godbolt.org

To short circuit finding which instructions to use, I would first write what I wanted to achieve in equivalent C++ into Compiler Explorer and then look at the compiled instructions to piece together what to do.

https://www.intel.com/content/www/us/en/developer/articles/technical/intel-sdm.html

Intel 64 and IA-32 Architectures Software Developer's Manual was ace at describing instructions from the POV of the bytes required. No way I read it through, just read what I needed as I worked.
