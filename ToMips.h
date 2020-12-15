#ifndef COMPILER_0_TOMIPS_H
#define COMPILER_0_TOMIPS_H

#include <fstream>
#include "MidCode.h"

#define $zero 0
#define $at 1
#define $v0 2
#define $v1 3 // SET INIT VAL OF LOCAL VAR
#define $a0 4
#define $a1 5
#define $a2 6
#define $a3 7
#define $t0 8
#define $t1 9
#define $t2 10
#define $t3 11 // READ
#define $t4 12 // ARRAY SUB
#define $t5 13 // RETURN
#define $t6 14
#define $t7 15
#define $s0 16
#define $s1 17
#define $s2 18
#define $s3 19
#define $s4 20
#define $s5 21
#define $s6 22
#define $s7 23
#define $t8 24
#define $t9 24
#define $k0 26
#define $k1 27
#define $gp 28
#define $sp 29
#define $s8 30
#define $ra 31

extern std::ofstream mipsFile;

void toMips();

#endif //COMPILER_0_TOMIPS_H
