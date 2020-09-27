cmake_minimum_required(VERSION 3.17)
project(compiler_0 CXX)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_CXX_FLAGS "-g")

add_executable(compiler_lexer main.cpp lexer.cpp lexer.h)
