#include "functions.h"
#include "float40.h"
#include <cmath>
#include <iomanip>
#include <sstream>
#ifndef _WIN32
#include <termios.h>
#include <unistd.h>
#endif

Value funcSin(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.sin().toDouble());
}

Value funcCos(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.cos().toDouble());
}

Value funcTan(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.tan().toDouble());
}

Value funcAtn(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.atn().toDouble());
}

Value funcExp(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.exp().toDouble());
}

Value funcLog(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.log().toDouble());
}

Value funcSqr(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.sqr().toDouble());
}

Value funcAbs(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.abs().toDouble());
}

Value funcInt(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.intPart().toDouble());
}

Value funcSgn(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.sgn().toDouble());
}

Value funcRnd(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(Float40::rnd(f).toDouble());
}

Value funcLen(const Value &arg) {
  return Value(static_cast<double>(arg.getString().length()));
}

Value funcVal(const Value &arg) {
  std::string str = arg.getString();
  try {
    return Value(std::stod(str));
  } catch (...) {
    return Value(0.0);
  }
}

Value funcAsc(const Value &arg) {
  std::string str = arg.getString();
  if (str.empty()) {
    throw std::runtime_error("ILLEGAL QUANTITY ERROR");
  }
  return Value(static_cast<double>(static_cast<unsigned char>(str[0])));
}

Value funcChr(const Value &arg) {
  int code = static_cast<int>(arg.getNumber());
  if (code < 0 || code > 255) {
    throw std::runtime_error("ILLEGAL QUANTITY ERROR");
  }
  return Value(std::string(1, static_cast<char>(code)));
}

Value funcLeft(const Value &str, const Value &len) {
  std::string s = str.getString();
  int n = static_cast<int>(len.getNumber());
  if (n < 0)
    n = 0;
  if (n > static_cast<int>(s.length()))
    n = s.length();
  return Value(s.substr(0, n));
}

Value funcRight(const Value &str, const Value &len) {
  std::string s = str.getString();
  int n = static_cast<int>(len.getNumber());
  if (n < 0)
    n = 0;
  if (n > static_cast<int>(s.length()))
    n = s.length();
  return Value(s.substr(s.length() - n, n));
}

Value funcMid(const Value &str, const Value &start, const Value &len) {
  std::string s = str.getString();
  int st = static_cast<int>(start.getNumber()) - 1; // BASIC is 1-indexed
  int ln = static_cast<int>(len.getNumber());

  if (st < 0)
    st = 0;
  if (st >= static_cast<int>(s.length()))
    return Value("");
  if (ln < 0)
    ln = 0;

  return Value(s.substr(st, ln));
}

Value funcStr(const Value &arg) {
  Float40 f(arg.getNumber());
  std::string result = f.toString();
  if (result[0] != '-') {
    result = " " + result; // Positive numbers get leading space
  }
  return Value(result);
}

Value funcTab(const Value &arg) {
  int n = static_cast<int>(arg.getNumber());
  if (n < 0)
    n = 0;
  return Value(std::string(static_cast<size_t>(n), ' '));
}

Value funcSpc(const Value &arg) {
  int n = static_cast<int>(arg.getNumber());
  if (n < 0)
    n = 0;
  return Value(std::string(static_cast<size_t>(n), ' '));
}

Value funcPos(const Value &) {
  int col = -1;

#ifndef _WIN32
  if (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
    termios oldt;
    if (tcgetattr(STDIN_FILENO, &oldt) == 0) {
      termios raw = oldt;
      raw.c_lflag &= static_cast<unsigned>(~(ICANON | ECHO));
      raw.c_cc[VMIN] = 0;
      raw.c_cc[VTIME] = 1; // Tenths of a second

      if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) == 0) {
        const char query[] = "\x1b[6n";
        (void)!write(STDOUT_FILENO, query, sizeof(query) - 1);

        char buf[32];
        ssize_t n = read(STDIN_FILENO, buf, sizeof(buf) - 1);
        if (n > 0) {
          buf[n] = '\0';
          int row = 0;
          int parsedCol = 0;
          if (sscanf(buf, "\x1b[%d;%dR", &row, &parsedCol) == 2) {
            col = parsedCol;
          }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
      }
    }
  }
#endif

  if (col <= 0) {
    return Value(0.0);
  }
  return Value(static_cast<double>(col - 1)); // BASIC columns are 0-based
}
