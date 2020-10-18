#ifndef COMPILER_0_PARSER_H
#define COMPILER_0_PARSER_H

//class parser {
//
//};
//
//// Parser will start with program()
void program(); // 11
//
//// program:
//static void constDec(); // 18
//static void varDec(); // 21
//static void returnFuncDef(); // 28
//static void nonReturnFuncDef(); // 33
//static void mainFunc(); // Native: compoundStat()
//
//// constDec:
//static void constDef(); // Native: int, <IDENFR>, integer(), char, character()
//
//// varDec:
//static void varDef(); // 25
//
//// varDef:
//static void varDefNoInit(); // Native: typeSpec(), <IDENFR>, [], unsignedInteger()
//static void varDefWithInit(); // Native: typeSpec(), <IDENFR>, [], unsignedInteger(), constExpr()
//
//// returnFuncDef:
//static void defHead(); // Native: int|char <IDENFR>
//static void paramTable(); // 39
//static void compoundStat(); // 41
//
//// nonReturnFuncDef:
////static void defHead(); // Native: void <IDENFR>
////static void paramTable(); // 38
////static void compoundStat();
//
//// paramTable: <IDENFR>
//static void typeSpec(); // Native: int|char
//
//// compoundStat:
////static void constDec();
////static void varDec();
//static void stats(); // 46
//
//// stats:
//static void stat(); //49
//
//// stat:
//static void loopStat(); // 61
//static void ifStat();// Native: condition(), stat()
//static void returnCallStat();// 67
//static void nonReturnCallStat();// 67
//static void assignStat();// Native: <IDENFR>, expr()
//static void readStat();// Native: scanf, <IDENFR>
//static void writeStat();// Native: printf, <STRCON>, expr()
//static void caseStat();// 70
//static void returnStat();// Native: expr()
//// { static void stats() } // 46
//
//// loopStat:
//static void whileLoopStat(); // Native: condition(), stat()
//static void forLoopStat(); // Native: condition(), <IDENFR>, expr(), +|-, step()
//static void condition(); // Native: expr(), relationOp()
//static void step(); // Native: unsignedInteger()
//
//// returnCallState, nonReturnCallState:
//static void valueTable(); // Native: expr()
//
//// caseStat: expr()
//static void caseTable(); // Native: caseSubStat()
//static void caseSubStat(); // Native: constExpr(), stat()
//static void defaultStat(); // Native: stat()
//
//// Common Usage
//static void integer();// Native: +|- unsignedInteger()
//static void unsignedInteger();// Native: <INTCON>
//static void character(); // Native: <CHARCON>
//static void expr(); // Native: +|-, term()
//static void term(); // Native: factor(), * /
//static void factor(); // Native: <IDENFR>, [], integer(), character(), returnCallStat()
//static void relationOp(); // Native: < | <= | > | >= | != | ==

#endif //COMPILER_0_PARSER_H
