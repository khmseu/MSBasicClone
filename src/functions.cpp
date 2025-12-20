#include "functions.h"
#include "float40.h"
#include "graphics.h"
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif
#include <unordered_map>

namespace {
std::unordered_map<int, int> &memoryMap() {
  static std::unordered_map<int, int> mem;
  return mem;
}
// Memory bounds used by peek/poke and WAIT; defaults align with interpreter.
static int gLomem = 2048;  // $0800
static int gHimem = 49152; // $C000
} // namespace

void pokeMemory(int addr, int val) {
  auto &mem = memoryMap();
  
  // Handle negative addresses (Apple II convention)
  if (addr < 0) {
    addr = 65536 + addr;  // Convert negative to 16-bit unsigned
  }
  
  // Special addresses that bypass range checks and have special behavior
  
  // Memory pointers and error handling (0-255 range)
  if (addr == 37 || addr == 105 || addr == 106 || 
      addr == 115 || addr == 116 || addr == 216 || 
      addr == 218 || addr == 219 || addr == 222) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Text window control (32-37)
  if (addr >= 32 && addr <= 37) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Shape table pointers (232-233)
  if (addr == 232 || addr == 233) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Hi-res page pointers (103-104)
  if (addr == 103 || addr == 104) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Graphics memory base addresses
  if (addr == 16384 || addr == 24576) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Keyboard strobe (49168)
  if (addr == 49168 || addr == -16368) {
    // Clear keyboard strobe - no-op in our implementation
    return;
  }
  
  // Display control switches (49232-49239)
  if (addr >= 49232 && addr <= 49239) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Annunciator outputs (49240-49247)
  if (addr >= 49240 && addr <= 49247) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Standard memory range check
  if (addr < gLomem || addr > gHimem) {
    throw std::runtime_error("MEMORY RANGE ERROR");
  }
  mem[addr] = val & 0xFF;
}

int peekMemory(int addr) {
  auto &mem = memoryMap();
  
  // Handle negative addresses (Apple II convention)
  if (addr < 0) {
    addr = 65536 + addr;  // Convert negative to 16-bit unsigned
  }
  
  // Special addresses that bypass range checks
  
  // Keyboard input (49152 / -16384)
  if (addr == 49152 || addr == -16384) {
    // Return last key pressed (stub - would need actual keyboard state)
    return mem.count(49152) ? mem[49152] : 0;
  }
  
  // Button inputs (49249-49251 / -16287 to -16285)
  if (addr >= 49249 && addr <= 49251) {
    // Return button state (stub - always unpressed)
    return 0;
  }
  
  // Memory pointers and error handling
  if (addr == 37 || addr == 105 || addr == 106 || 
      addr == 115 || addr == 116 || addr == 216 || 
      addr == 218 || addr == 219 || addr == 222) {
    auto it = mem.find(addr);
    if (it == mem.end()) {
      return 0;
    }
    return it->second & 0xFF;
  }
  
  // Text window control (32-37)
  if (addr >= 32 && addr <= 37) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Shape table pointers (232-233)
  if (addr == 232 || addr == 233) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Hi-res page pointers (103-104)
  if (addr == 103 || addr == 104) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Graphics memory base addresses
  if (addr == 16384 || addr == 24576) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Display control switches (49232-49239)
  if (addr >= 49232 && addr <= 49239) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Annunciator outputs (49240-49247)
  if (addr >= 49240 && addr <= 49247) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Standard memory range check
  if (addr < gLomem || addr > gHimem) {
    throw std::runtime_error("MEMORY RANGE ERROR");
  }
  auto it = mem.find(addr);
  if (it == mem.end()) {
    return 0;
  }
  return it->second & 0xFF;
}

void setMemoryBounds(int lomem, int himem) {
  // Ensure sane ordering; if invalid, keep existing.
  if (lomem <= himem) {
    gLomem = lomem;
    gHimem = himem;
  }
}

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

#ifdef _WIN32
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  if (h != nullptr && h != INVALID_HANDLE_VALUE) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(h, &info)) {
      col = static_cast<int>(info.dwCursorPosition.X);
    }
  }
#else
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
            col = parsedCol - 1; // Escape reports 1-based column
          }
        }

        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
      }
    }
  }
#endif

  if (col < 0) {
    return Value(0.0);
  }
  return Value(static_cast<double>(col));
}

Value funcFre(const Value &) {
  // Placeholder free-memory report; Applesoft returned bytes free. Use fixed
  // value to keep behavior deterministic across platforms.
  return Value(32767.0);
}

Value funcPdl(const Value &) {
  // Paddle input not supported; return 0.
  return Value(0.0);
}

Value funcPeek(const Value &arg) {
  int addr = static_cast<int>(arg.getNumber());
  int result = peekMemory(addr);
  return Value(static_cast<double>(result));
}
Value funcScrn(const Value &x, const Value &y) {
  double dx = x.getNumber();
  double dy = y.getNumber();
  int color = graphics().scrn(dx, dy);
  return Value(static_cast<double>(color));
}

Value funcUsr(const Value &addr) {
  // USR(addr) calls machine language code at address
  // Stub implementation: return 0
  (void)addr;
  return Value(0.0);
}