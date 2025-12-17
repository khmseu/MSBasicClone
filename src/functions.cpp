#include "functions.h"
#include "float40.h"
#include <cmath>
#include <sstream>
#include <iomanip>

Value funcSin(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.sin().toDouble());
}

Value funcCos(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.cos().toDouble());
}

Value funcTan(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.tan().toDouble());
}

Value funcAtn(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.atn().toDouble());
}

Value funcExp(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.exp().toDouble());
}

Value funcLog(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.log().toDouble());
}

Value funcSqr(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.sqr().toDouble());
}

Value funcAbs(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.abs().toDouble());
}

Value funcInt(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.intPart().toDouble());
}

Value funcSgn(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(f.sgn().toDouble());
}

Value funcRnd(const Value& arg) {
    Float40 f(arg.getNumber());
    return Value(Float40::rnd(f).toDouble());
}

Value funcLen(const Value& arg) {
    return Value(static_cast<double>(arg.getString().length()));
}

Value funcVal(const Value& arg) {
    std::string str = arg.getString();
    try {
        return Value(std::stod(str));
    } catch (...) {
        return Value(0.0);
    }
}

Value funcAsc(const Value& arg) {
    std::string str = arg.getString();
    if (str.empty()) {
        throw std::runtime_error("ILLEGAL QUANTITY ERROR");
    }
    return Value(static_cast<double>(static_cast<unsigned char>(str[0])));
}

Value funcChr(const Value& arg) {
    int code = static_cast<int>(arg.getNumber());
    if (code < 0 || code > 255) {
        throw std::runtime_error("ILLEGAL QUANTITY ERROR");
    }
    return Value(std::string(1, static_cast<char>(code)));
}

Value funcLeft(const Value& str, const Value& len) {
    std::string s = str.getString();
    int n = static_cast<int>(len.getNumber());
    if (n < 0) n = 0;
    if (n > static_cast<int>(s.length())) n = s.length();
    return Value(s.substr(0, n));
}

Value funcRight(const Value& str, const Value& len) {
    std::string s = str.getString();
    int n = static_cast<int>(len.getNumber());
    if (n < 0) n = 0;
    if (n > static_cast<int>(s.length())) n = s.length();
    return Value(s.substr(s.length() - n, n));
}

Value funcMid(const Value& str, const Value& start, const Value& len) {
    std::string s = str.getString();
    int st = static_cast<int>(start.getNumber()) - 1; // BASIC is 1-indexed
    int ln = static_cast<int>(len.getNumber());
    
    if (st < 0) st = 0;
    if (st >= static_cast<int>(s.length())) return Value("");
    if (ln < 0) ln = 0;
    
    return Value(s.substr(st, ln));
}

Value funcStr(const Value& arg) {
    Float40 f(arg.getNumber());
    std::string result = f.toString();
    if (result[0] != '-') {
        result = " " + result;  // Positive numbers get leading space
    }
    return Value(result);
}

Value funcTab(const Value& arg) {
    int n = static_cast<int>(arg.getNumber());
    if (n < 0) n = 0;
    return Value(std::string(static_cast<size_t>(n), ' '));
}

Value funcSpc(const Value& arg) {
    int n = static_cast<int>(arg.getNumber());
    if (n < 0) n = 0;
    return Value(std::string(static_cast<size_t>(n), ' '));
}

Value funcPos(const Value&) {
    // No real cursor tracking; return 0 to indicate start of line.
    return Value(0.0);
}
