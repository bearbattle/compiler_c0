cmake_minimum_required(VERSION 3.16)
project(compiler_0)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

#add_executable(compiler_lexer main.cpp lexer.cpp lexer.h)

add_executable(compiler main.cpp lexer.cpp parser.cpp parser.cpp SymbolTable.cpp)