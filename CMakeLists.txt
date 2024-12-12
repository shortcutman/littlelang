cmake_minimum_required(VERSION 3.10)

project(LittleLang)

set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


add_executable(LittleLang main.cpp)
target_compile_options(LittleLang PRIVATE -masm=intel)