#pragma once

#include "types.h"
#include <string>

// Built-in mathematical functions
Value funcSin(const Value &arg);
Value funcCos(const Value &arg);
Value funcTan(const Value &arg);
Value funcAtn(const Value &arg);
Value funcExp(const Value &arg);
Value funcLog(const Value &arg);
Value funcSqr(const Value &arg);
Value funcAbs(const Value &arg);
Value funcInt(const Value &arg);
Value funcSgn(const Value &arg);
Value funcRnd(const Value &arg);

// String functions
Value funcLen(const Value &arg);
Value funcVal(const Value &arg);
Value funcAsc(const Value &arg);
Value funcChr(const Value &arg);
Value funcLeft(const Value &str, const Value &len);
Value funcRight(const Value &str, const Value &len);
Value funcMid(const Value &str, const Value &start, const Value &len);
Value funcStr(const Value &arg);
Value funcTab(const Value &arg);
Value funcSpc(const Value &arg);
Value funcPos(const Value &arg);
Value funcFre(const Value& arg);
Value funcPdl(const Value& arg);
Value funcPeek(const Value& arg);

// Simplified memory model used by PEEK/POKE helpers.
void pokeMemory(int addr, int val);
int peekMemory(int addr);
