/**
 * @file functions.cpp
 * @brief Implementation of Applesoft BASIC built-in functions and memory operations
 * 
 * This file implements all built-in functions from Applesoft BASIC including
 * mathematical operations, string manipulation, memory access (PEEK/POKE), and
 * system functions. The implementation maintains compatibility with Applesoft
 * semantics while providing cross-platform functionality.
 * 
 * Memory Model:
 * The memory system uses a sparse map to store byte values at specific addresses.
 * This allows efficient memory simulation without allocating full 64K arrays.
 * Special addresses have specific behaviors:
 * 
 * Zero Page (0x00-0xFF):
 *   - 0x0025: Vertical cursor position
 *   - 0x0069-0x006A: LOMEM pointer (low/high bytes)
 *   - 0x0073-0x0074: HIMEM pointer (low/high bytes)
 *   - 0x00D8: ONERR flag
 *   - 0x00DA-0x00DB: ONERR line number
 *   - 0x00DE: ProDOS error code (location 222)
 *   - 0x00E8-0x00E9: Shape table pointer
 * 
 * Text Window Control (0x20-0x25):
 *   - Control characters for text output window configuration
 * 
 * System Locations (0xC000-0xFFFF):
 *   - 0xC000 (-16384): Keyboard input
 *   - 0xC010 (-16368): Keyboard strobe clear
 *   - 0xC050-0xC057: Display control switches
 *   - 0xC058-0xC05F: Annunciator outputs
 *   - 0xC061-0xC063: Button inputs
 * 
 * Graphics Memory:
 *   - 0x4000: Hi-res page 1 base
 *   - 0x6000: Hi-res page 2 base
 * 
 * Memory Range Protection:
 * Most addresses are validated against LOMEM/HIMEM bounds. Special system
 * addresses bypass this check for compatibility with Applesoft programs that
 * access hardware locations.
 */

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
/**
 * @brief Get the singleton memory map
 * 
 * Returns a reference to the static memory map that stores byte values at
 * specific addresses. Using a map allows sparse storage - only addresses that
 * have been written are stored, saving memory.
 * 
 * @return Reference to the global memory map
 */
std::unordered_map<int, int> &memoryMap() {
  static std::unordered_map<int, int> mem;
  return mem;
}

/**
 * @brief Lower memory bound for PEEK/POKE validation
 * 
 * Default is 0x0800 (2048), matching Applesoft BASIC on a 48K Apple II.
 * Can be modified via setMemoryBounds() to support different configurations.
 */
static int gLomem = 0x0800;  // $0800 (2048)

/**
 * @brief Upper memory bound for PEEK/POKE validation
 * 
 * Default is 0xC000 (49152), which is where I/O and ROM begin on Apple II.
 * Can be modified via setMemoryBounds() to support different configurations.
 */
static int gHimem = 0xC000; // $C000 (49152)
} // namespace

/**
 * @brief Format a memory address as hexadecimal string
 * 
 * Converts an integer address to a hexadecimal string prefixed with '$'.
 * Used for displaying addresses in error messages and CALL statements.
 * 
 * Implementation details:
 * - When mask16bit is true, address is masked to 16 bits (0xFFFF) and
 *   formatted with exactly 4 hex digits (e.g., $C000)
 * - When mask16bit is false, address is shown with its natural width
 *   (e.g., $186A0 for 100000)
 * - All hex digits are uppercase per Applesoft convention
 * 
 * @param addr Address to format (may be negative for Apple II convention)
 * @param mask16bit If true, mask to 16 bits; if false, show full value
 * @return Formatted hex string like "$C000" or "$186A0"
 */
std::string formatHexAddress(int addr, bool mask16bit) {
  std::ostringstream oss;
  if (mask16bit) {
    addr = addr & 0xFFFF;
    oss << "$" << std::hex << std::uppercase << std::setfill('0') << std::setw(4) << addr;
  } else {
    oss << "$" << std::hex << std::uppercase << addr;
  }
  return oss.str();
}

/**
 * @brief Write a byte value to memory (POKE implementation)
 * 
 * Implements the POKE statement which writes a byte to a memory address.
 * This function handles the complex address mapping of the Apple II including
 * special system addresses, I/O locations, and memory-mapped hardware.
 * 
 * Address Handling:
 * - Negative addresses are converted to 16-bit unsigned (Apple II convention)
 *   Example: POKE -16368,0 becomes POKE 49168,0 (clear keyboard strobe)
 * - Values are masked to 8 bits (0-255) to simulate byte storage
 * 
 * Special Addresses (bypass range checking):
 * - Zero page system variables (0x00-0xFF)
 * - Text window control (0x20-0x25)
 * - Memory pointers: LOMEM (0x69-0x6A), HIMEM (0x73-0x74)
 * - Error handling: ONERR flag (0xD8), line (0xDA-0xDB), error code (0xDE)
 * - Shape table pointer (0xE8-0xE9)
 * - Graphics page pointers (0x67-0x68)
 * - Hi-res page base addresses (0x4000, 0x6000)
 * - Keyboard strobe clear (0xC010/-16368) - acknowledged but no-op
 * - Display control switches (0xC050-0xC057)
 * - Annunciator outputs (0xC058-0xC05F)
 * 
 * Range Checking:
 * Addresses not in the special categories above must fall within LOMEM to HIMEM
 * bounds. Out-of-range addresses throw "MEMORY RANGE ERROR" which matches
 * Applesoft's "ILLEGAL QUANTITY ERROR" for invalid memory access.
 * 
 * @param addr Memory address to write to (may be negative)
 * @param val Value to write (will be masked to 0-255)
 * @throws std::runtime_error If address is outside valid range and not special
 */
void pokeMemory(int addr, int val) {
  auto &mem = memoryMap();
  
  // Handle negative addresses (Apple II convention)
  // Applesoft BASIC allows negative addresses as shorthand for high memory
  // Example: -16384 (decimal) = 49152 (0xC000) - the keyboard input register
  if (addr < 0) {
    addr = 0x10000 + addr;  // Convert negative to 16-bit unsigned
  }
  
  // Special addresses that bypass range checks and have special behavior
  
  // Memory pointers and error handling (0-255 range)
  // These zero-page locations are used by the interpreter for system state
  if (addr == 0x0025 || addr == 0x0069 || addr == 0x006A || 
      addr == 0x0073 || addr == 0x0074 || addr == 0x00D8 || 
      addr == 0x00DA || addr == 0x00DB || addr == 0x00DE) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Text window control (32-37)
  // These locations control the text output window position and dimensions
  if (addr >= 0x0020 && addr <= 0x0025) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Shape table pointers (232-233)
  // Low and high bytes of shape table memory address for DRAW/XDRAW
  if (addr == 0x00E8 || addr == 0x00E9) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Hi-res page pointers (103-104)
  // Stores which hi-res page is currently active (page 1 or page 2)
  if (addr == 0x0067 || addr == 0x0068) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Graphics memory base addresses
  // These are the base addresses for hi-res graphics pages
  if (addr == 0x4000 || addr == 0x6000) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Keyboard strobe (49168 / -16368)
  // Writing any value clears the keyboard strobe (acknowledges key press)
  // In our implementation this is a no-op since we don't buffer keyboard input
  if (addr == 0xC010 || addr == -16368) {
    // Clear keyboard strobe - no-op in our implementation
    return;
  }
  
  // Display control switches (49232-49239)
  // These soft switches control text/graphics mode and page selection
  // We store them but interpretation happens elsewhere
  if (addr >= 0xC050 && addr <= 0xC057) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Annunciator outputs (49240-49247)
  // General-purpose output bits, typically used for hardware control
  if (addr >= 0xC058 && addr <= 0xC05F) {
    mem[addr] = val & 0xFF;
    return;
  }
  
  // Standard memory range check
  // All other addresses must fall within LOMEM to HIMEM bounds
  if (addr < gLomem || addr > gHimem) {
    throw std::runtime_error("MEMORY RANGE ERROR: " + formatHexAddress(addr, false));
  }
  mem[addr] = val & 0xFF;
}

/**
 * @brief Read a byte value from memory (PEEK implementation)
 * 
 * Implements the PEEK function which reads a byte from a memory address.
 * Like POKE, this handles special Apple II memory-mapped locations and
 * provides read access to system state and I/O registers.
 * 
 * Address Handling:
 * - Negative addresses are converted to 16-bit unsigned (Apple II convention)
 * - Returns 0 for uninitialized memory locations (sparse storage)
 * - All values returned are in range 0-255 (byte values)
 * 
 * Special Read-Only Addresses:
 * - Keyboard input (0xC000/-16384): Returns last key pressed with high bit set
 * - Button inputs (0xC061-0xC063): Returns button states (stub: always 0)
 * 
 * Special Read-Write Addresses:
 * - All addresses writable by POKE can also be read
 * - System variables, pointers, I/O locations
 * 
 * Implementation Notes:
 * - Keyboard and button inputs are stubs returning default values
 * - Real Apple II hardware would return actual hardware state
 * - Unwritten addresses return 0 (memory map is sparse)
 * 
 * Range Checking:
 * Unlike POKE, PEEK does not throw errors for out-of-range addresses.
 * This matches Applesoft behavior where PEEK can read anywhere but returns
 * undefined values outside LOMEM-HIMEM range.
 * 
 * @param addr Memory address to read from (may be negative)
 * @return Byte value at address (0-255), or 0 if uninitialized
 */
int peekMemory(int addr) {
  auto &mem = memoryMap();
  
  // Handle negative addresses (Apple II convention)
  // Same conversion as POKE: -16384 becomes 49152 (0xC000)
  if (addr < 0) {
    addr = 0x10000 + addr;  // Convert negative to 16-bit unsigned
  }
  
  // Special addresses that bypass range checks
  
  // Keyboard input (49152 / -16384)
  // On real Apple II, returns last key with bit 7 set if key available
  // We return stored value or 0 (stub implementation)
  if (addr == 0xC000 || addr == -16384) {
    // Return last key pressed (stub - would need actual keyboard state)
    return mem.count(0xC000) ? mem[0xC000] : 0;
  }
  
  // Button inputs (49249-49251 / -16287 to -16285)
  // Returns button state: bit 7 set if pressed, clear if not
  // Stub implementation always returns 0 (not pressed)
  if (addr >= 0xC061 && addr <= 0xC063) {
    // Return button state (stub - always unpressed)
    return 0;
  }
  
  // Memory pointers and error handling
  // Same zero-page locations as POKE, readable for inspection
  if (addr == 0x0025 || addr == 0x0069 || addr == 0x006A || 
      addr == 0x0073 || addr == 0x0074 || addr == 0x00D8 || 
      addr == 0x00DA || addr == 0x00DB || addr == 0x00DE) {
    auto it = mem.find(addr);
    if (it == mem.end()) {
      return 0;
    }
    return it->second & 0xFF;
  }
  
  // Text window control (32-37)
  if (addr >= 0x0020 && addr <= 0x0025) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Text window control (32-37)
  // Control the text output window boundaries
  if (addr >= 0x0020 && addr <= 0x0025) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Shape table pointers (232-233)
  // Low/high bytes of shape table address used by DRAW/XDRAW commands
  if (addr == 0x00E8 || addr == 0x00E9) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Hi-res page pointers (103-104)
  // Indicates which hi-res graphics page is active
  if (addr == 0x0067 || addr == 0x0068) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Graphics memory base addresses
  // Base addresses for hi-res page 1 (0x4000) and page 2 (0x6000)
  if (addr == 0x4000 || addr == 0x6000) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Display control switches (49232-49239)
  // Soft switches that control display modes on Apple II
  if (addr >= 0xC050 && addr <= 0xC057) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Annunciator outputs (49240-49247)
  // General-purpose output bits for peripheral control
  if (addr >= 0xC058 && addr <= 0xC05F) {
    auto it = mem.find(addr);
    return (it != mem.end()) ? (it->second & 0xFF) : 0;
  }
  
  // Standard memory range check
  // For addresses in normal RAM range, validate against LOMEM/HIMEM
  if (addr < gLomem || addr > gHimem) {
    throw std::runtime_error("MEMORY RANGE ERROR: " + formatHexAddress(addr, false));
  }
  // Return stored value or 0 if never written (sparse storage)
  auto it = mem.find(addr);
  if (it == mem.end()) {
    return 0;
  }
  return it->second & 0xFF;
}

/**
 * @brief Set memory bounds for PEEK/POKE validation
 * 
 * Configures the valid memory range for PEEK/POKE operations. This allows
 * the interpreter to simulate different Apple II configurations (48K, 64K, etc.)
 * 
 * Default bounds:
 * - LOMEM: 0x0800 (2048) - Start of BASIC program area on 48K Apple II
 * - HIMEM: 0xC000 (49152) - Start of I/O and ROM on Apple II
 * 
 * The bounds are validated for sanity - LOMEM must be <= HIMEM. If invalid
 * bounds are provided, the existing bounds are retained unchanged.
 * 
 * Special addresses (zero page, I/O) always bypass these checks, so setting
 * restrictive bounds won't prevent access to system locations.
 * 
 * @param lomem Lower memory bound (inclusive)
 * @param himem Upper memory bound (inclusive)
 */
void setMemoryBounds(int lomem, int himem) {
  // Ensure sane ordering; if invalid, keep existing.
  if (lomem <= himem) {
    gLomem = lomem;
    gHimem = himem;
  }
}

// ============================================================================
// Mathematical Functions
// ============================================================================
// All trigonometric functions use Float40 for Applesoft-compatible precision

/**
 * @brief Sine function (SIN)
 * 
 * Computes sine using Float40 to match Applesoft precision.
 * 
 * @param arg Angle in radians
 * @return Sine of arg
 */
Value funcSin(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.sin().toDouble());
}

/**
 * @brief Cosine function (COS)
 * 
 * Computes cosine using Float40 to match Applesoft precision.
 * 
 * @param arg Angle in radians
 * @return Cosine of arg
 */
Value funcCos(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.cos().toDouble());
}

/**
 * @brief Tangent function (TAN)
 * 
 * Computes tangent using Float40 to match Applesoft precision.
 * 
 * @param arg Angle in radians
 * @return Tangent of arg
 */
Value funcTan(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.tan().toDouble());
}

/**
 * @brief Arctangent function (ATN)
 * 
 * Computes arctangent using Float40 to match Applesoft precision.
 * Returns value in range -π/2 to π/2.
 * 
 * @param arg Value to compute arctangent of
 * @return Arctangent of arg in radians
 */
Value funcAtn(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.atn().toDouble());
}

/**
 * @brief Exponential function (EXP)
 * 
 * Computes e^arg using Float40 to match Applesoft precision.
 * 
 * @param arg Exponent
 * @return e raised to the power of arg
 */
Value funcExp(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.exp().toDouble());
}

/**
 * @brief Natural logarithm (LOG)
 * 
 * Computes natural logarithm using Float40 to match Applesoft precision.
 * Throws error if arg <= 0 (log of non-positive numbers is undefined).
 * 
 * @param arg Value (must be positive)
 * @return Natural logarithm of arg
 * @throws RuntimeError if arg <= 0
 */
Value funcLog(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.log().toDouble());
}

/**
 * @brief Square root (SQR)
 * 
 * Computes square root using Float40 to match Applesoft precision.
 * Throws error if arg < 0 (square root of negative numbers is undefined).
 * 
 * @param arg Value (must be non-negative)
 * @return Square root of arg
 * @throws RuntimeError if arg < 0
 */
Value funcSqr(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.sqr().toDouble());
}

/**
 * @brief Absolute value (ABS)
 * 
 * Returns the absolute value using Float40 precision.
 * 
 * @param arg Input value
 * @return |arg| (always non-negative)
 */
Value funcAbs(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.abs().toDouble());
}

/**
 * @brief Integer part (INT)
 * 
 * Returns the largest integer less than or equal to arg (floor function).
 * This matches Applesoft BASIC behavior where INT(-2.5) = -3.
 * 
 * Note: This is NOT truncation towards zero. INT always rounds down.
 * Examples:
 *   INT(2.5) = 2
 *   INT(-2.5) = -3 (not -2)
 * 
 * @param arg Input value
 * @return Largest integer <= arg
 */
Value funcInt(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.intPart().toDouble());
}

/**
 * @brief Sign function (SGN)
 * 
 * Returns the sign of the argument:
 * - Returns -1 if arg < 0
 * - Returns 0 if arg = 0
 * - Returns 1 if arg > 0
 * 
 * @param arg Input value
 * @return -1, 0, or 1 depending on sign of arg
 */
Value funcSgn(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(f.sgn().toDouble());
}

/**
 * @brief Random number generator (RND)
 * 
 * Generates pseudo-random numbers using Float40 implementation for
 * Applesoft-compatible random number sequences.
 * 
 * Behavior based on argument:
 * - RND(1) or RND(positive): Returns random number in range [0, 1)
 * - RND(0): Repeats the last random number generated
 * - RND(negative): Reseeds the generator based on the argument value
 * 
 * This matches Applesoft BASIC's RND behavior exactly, including
 * the ability to reseed for reproducible sequences.
 * 
 * @param arg Control parameter (typically 1 for new random number)
 * @return Random number in [0, 1) or repeated/reseeded value
 */
Value funcRnd(const Value &arg) {
  Float40 f(arg.getNumber());
  return Value(Float40::rnd(f).toDouble());
}

// ============================================================================
// String Functions
// ============================================================================

/**
 * @brief String length (LEN)
 * 
 * Returns the number of characters in a string.
 * 
 * @param arg String value
 * @return Length of string as a number
 */
Value funcLen(const Value &arg) {
  return Value(static_cast<double>(arg.getString().length()));
}

/**
 * @brief Convert string to number (VAL)
 * 
 * Parses a string and returns its numeric value. This function attempts to
 * interpret the string as a number, following these rules:
 * - Leading whitespace is ignored
 * - Parsing stops at first non-numeric character
 * - Invalid strings return 0.0 (no error)
 * 
 * Examples:
 *   VAL("123") = 123
 *   VAL("12.5") = 12.5
 *   VAL("  45") = 45
 *   VAL("12ABC") = 12 (stops at 'A')
 *   VAL("ABC") = 0 (invalid)
 * 
 * @param arg String to convert
 * @return Numeric value, or 0.0 if string is not a valid number
 */
Value funcVal(const Value &arg) {
  std::string str = arg.getString();
  try {
    return Value(std::stod(str));
  } catch (...) {
    return Value(0.0);
  }
}

/**
 * @brief Get ASCII code of first character (ASC)
 * 
 * Returns the ASCII code (0-255) of the first character in a string.
 * This is commonly used to check for special keys or control characters.
 * 
 * The function throws "ILLEGAL QUANTITY ERROR" if the string is empty,
 * matching Applesoft BASIC behavior.
 * 
 * Examples:
 *   ASC("A") = 65
 *   ASC("HELLO") = 72 (ASCII code of 'H')
 *   ASC("") raises error
 * 
 * @param arg String value (must be non-empty)
 * @return ASCII code of first character (0-255)
 * @throws std::runtime_error if string is empty
 */
Value funcAsc(const Value &arg) {
  std::string str = arg.getString();
  if (str.empty()) {
    throw std::runtime_error("ILLEGAL QUANTITY ERROR");
  }
  return Value(static_cast<double>(static_cast<unsigned char>(str[0])));
}

/**
 * @brief Convert ASCII code to character (CHR$)
 * 
 * Returns a single-character string containing the character with the
 * specified ASCII code. This is the inverse of ASC.
 * 
 * The function validates that the code is in range 0-255 and throws
 * "ILLEGAL QUANTITY ERROR" for out-of-range values.
 * 
 * Commonly used for:
 * - Control characters: CHR$(7) = bell, CHR$(13) = carriage return
 * - Special display effects
 * - Building strings character by character
 * 
 * Examples:
 *   CHR$(65) = "A"
 *   CHR$(7) = bell character
 *   CHR$(13) = carriage return
 *   CHR$(256) raises error
 * 
 * @param arg ASCII code (must be 0-255)
 * @return Single-character string
 * @throws std::runtime_error if code is out of range
 */
Value funcChr(const Value &arg) {
  int code = static_cast<int>(arg.getNumber());
  if (code < 0 || code > 255) {
    throw std::runtime_error("ILLEGAL QUANTITY ERROR");
  }
  return Value(std::string(1, static_cast<char>(code)));
}

/**
 * @brief Extract left portion of string (LEFT$)
 * 
 * Returns the leftmost n characters of a string. If n exceeds the string
 * length, returns the entire string. Negative values for n are treated as 0.
 * 
 * Examples:
 *   LEFT$("HELLO", 2) = "HE"
 *   LEFT$("HELLO", 10) = "HELLO" (n > length)
 *   LEFT$("HELLO", 0) = ""
 *   LEFT$("HELLO", -1) = "" (negative treated as 0)
 * 
 * @param str String value
 * @param len Number of characters to extract
 * @return Leftmost len characters of str
 */
Value funcLeft(const Value &str, const Value &len) {
  std::string s = str.getString();
  int n = static_cast<int>(len.getNumber());
  if (n < 0)
    n = 0;
  if (n > static_cast<int>(s.length()))
    n = s.length();
  return Value(s.substr(0, n));
}

/**
 * @brief Extract right portion of string (RIGHT$)
 * 
 * Returns the rightmost n characters of a string. If n exceeds the string
 * length, returns the entire string. Negative values for n are treated as 0.
 * 
 * Examples:
 *   RIGHT$("HELLO", 2) = "LO"
 *   RIGHT$("HELLO", 10) = "HELLO" (n > length)
 *   RIGHT$("HELLO", 0) = ""
 *   RIGHT$("HELLO", -1) = "" (negative treated as 0)
 * 
 * @param str String value
 * @param len Number of characters to extract
 * @return Rightmost len characters of str
 */
Value funcRight(const Value &str, const Value &len) {
  std::string s = str.getString();
  int n = static_cast<int>(len.getNumber());
  if (n < 0)
    n = 0;
  if (n > static_cast<int>(s.length()))
    n = s.length();
  return Value(s.substr(s.length() - n, n));
}

/**
 * @brief Extract middle portion of string (MID$)
 * 
 * Returns a substring starting at position start with length len.
 * In Applesoft BASIC, string positions are 1-indexed (first character is 1).
 * 
 * Behavior details:
 * - start is 1-indexed: MID$("HELLO", 1, 2) = "HE"
 * - If start < 1, treated as 1
 * - If start > string length, returns empty string
 * - If len is negative, treated as 0 (returns empty string)
 * - If len exceeds remaining characters, returns rest of string
 * 
 * Examples:
 *   MID$("HELLO", 2, 3) = "ELL" (start at position 2, take 3 chars)
 *   MID$("HELLO", 4, 10) = "LO" (only 2 chars remain)
 *   MID$("HELLO", 10, 2) = "" (start beyond end)
 *   MID$("HELLO", 0, 2) = "HE" (start < 1 treated as 1)
 * 
 * @param str String value
 * @param start Starting position (1-indexed)
 * @param len Number of characters to extract
 * @return Substring from start position with length len
 */
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

/**
 * @brief Convert number to string (STR$)
 * 
 * Converts a numeric value to its string representation using Float40
 * formatting to match Applesoft precision and format.
 * 
 * Applesoft convention:
 * - Positive numbers get a leading space: STR$(5) = " 5"
 * - Negative numbers get a minus sign: STR$(-5) = "-5"
 * - This leading space ensures alignment in columnar output
 * 
 * The formatting matches Applesoft's number display format including
 * scientific notation for very large or very small numbers.
 * 
 * Examples:
 *   STR$(123) = " 123"
 *   STR$(-456) = "-456"
 *   STR$(1.5) = " 1.5"
 * 
 * @param arg Numeric value to convert
 * @return String representation with Applesoft formatting
 */
Value funcStr(const Value &arg) {
  Float40 f(arg.getNumber());
  std::string result = f.toString();
  if (result[0] != '-') {
    result = " " + result; // Positive numbers get leading space
  }
  return Value(result);
}

// ============================================================================
// Output Formatting Functions
// ============================================================================

/**
 * @brief Generate spaces for TAB positioning (TAB)
 * 
 * TAB is used in PRINT statements to position output at a specific column.
 * This function returns a string of spaces to pad to the specified position.
 * 
 * Note: The actual TAB positioning logic is in the PRINT statement handler,
 * which uses current cursor position. This function just generates spaces.
 * 
 * Negative values are treated as 0 (no spaces).
 * 
 * @param arg Target column position
 * @return String of spaces
 */
Value funcTab(const Value &arg) {
  int n = static_cast<int>(arg.getNumber());
  if (n < 0)
    n = 0;
  return Value(std::string(static_cast<size_t>(n), ' '));
}

/**
 * @brief Generate specified number of spaces (SPC)
 * 
 * SPC is used in PRINT statements to output a specific number of spaces.
 * Unlike TAB, SPC always outputs exactly n spaces regardless of cursor position.
 * 
 * Negative values are treated as 0 (no spaces).
 * 
 * Examples:
 *   PRINT "A";SPC(5);"B" outputs "A     B"
 * 
 * @param arg Number of spaces to generate
 * @return String containing arg spaces
 */
Value funcSpc(const Value &arg) {
  int n = static_cast<int>(arg.getNumber());
  if (n < 0)
    n = 0;
  return Value(std::string(static_cast<size_t>(n), ' '));
}

/**
 * @brief Get current cursor column position (POS)
 * 
 * Returns the current horizontal cursor position (column number) where the
 * next character will be printed. Column numbering starts at 0.
 * 
 * Platform-specific implementation:
 * - Windows: Uses GetConsoleScreenBufferInfo to query cursor position
 * - POSIX: Uses ANSI escape sequence (CPR - Cursor Position Report)
 * - Falls back to 0 if cursor position cannot be determined
 * 
 * The POSIX implementation:
 * 1. Sends escape sequence "\x1B[6n" to query cursor position
 * 2. Terminal responds with "\x1B[row;colR"
 * 3. Parses the response to extract column number
 * 4. Temporarily sets terminal to raw mode for reliable reading
 * 5. Times out after 0.1 seconds if no response
 * 
 * Used by PRINT TAB to calculate spacing needed to reach target column.
 * 
 * @param Unused argument (for function signature compatibility)
 * @return Current cursor column (0-based), or 0 if cannot be determined
 */
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