cmake_minimum_required(VERSION 3.17)
project(compiler_0 CXX)

set(CMAKE_CXX_STANDARD 98)

add_executable(compiler_0_1 testfile1.cpp myio.h myio.cpp)

add_executable(compiler_0_2 testfile2.cpp myio.h myio.cpp)

add_executable(compiler_0_3 testfile3.cpp myio.h myio.cpp)

add_executable(compiler_0_4 testfile4.cpp myio.h myio.cpp)