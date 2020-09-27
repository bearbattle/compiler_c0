cmake_minimum_required(VERSION 3.17)
project(compiler_0 CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(compiler_lexer main.cpp lexer.cpp lexer.h)
