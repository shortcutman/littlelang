cmake_minimum_required(VERSION 3.11)

project(LittleLang)

set(CMAKE_OSX_ARCHITECTURES "x86_64")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_COMPILE_WARNING_AS_ERROR True)

add_executable(LittleLang
    main.cpp
    Compilerx64.cpp
    InstrBufferx64.cpp
    Parser.cpp
)
target_compile_options(LittleLang PRIVATE -masm=intel)

include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    ll_tests
    Compilerx64.cpp
    Compilerx64.tests.cpp
    InstrBufferx64.cpp
    InstrBufferx64.tests.cpp
    Parser.cpp
    Parser.tests.cpp
)
target_link_libraries(
    ll_tests
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(ll_tests)
