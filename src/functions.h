/**
 * @file functions.h
 * @brief Applesoft BASIC built-in functions and memory operations
 * 
 * This file implements all built-in functions from Applesoft BASIC including:
 * - Mathematical functions (SIN, COS, TAN, ATN, EXP, LOG, SQR, ABS, INT, SGN)
 * - Random number generation (RND)
 * - String functions (LEN, VAL, ASC, CHR$, LEFT$, RIGHT$, MID$, STR$)
 * - Output formatting (TAB, SPC, POS)
 * - Memory operations (PEEK, POKE)
 * - System functions (FRE, PDL, SCRN, USR)
 * 
 * Memory Model:
 * The interpreter maintains a simplified memory map matching Applesoft conventions.
 * LOMEM and HIMEM define the valid memory range. Addresses outside this range
 * trigger "MEMORY RANGE ERROR" (matching ILLEGAL QUANTITY ERROR behavior).
 * 
 * Key memory locations:
 * - 222 ($DE): ProDOS error code
 * - 24 ($18): Horizontal cursor position
 * - Various other system locations for compatibility
 * 
 * Memory bounds are configurable via setMemoryBounds() to support different
 * memory configurations (e.g., 48K, 64K systems).
 */

#pragma once

#include "types.h"
#include <string>

// ============================================================================
// Mathematical Functions
// ============================================================================

/**
 * @brief Sine function (SIN)
 * @param arg Angle in radians
 * @return Sine of arg
 */
Value funcSin(const Value &arg);

/**
 * @brief Cosine function (COS)
 * @param arg Angle in radians
 * @return Cosine of arg
 */
Value funcCos(const Value &arg);

/**
 * @brief Tangent function (TAN)
 * @param arg Angle in radians
 * @return Tangent of arg
 */
Value funcTan(const Value &arg);

/**
 * @brief Arctangent function (ATN)
 * @param arg Value to compute arctangent of
 * @return Arctangent of arg in radians (-π/2 to π/2)
 */
Value funcAtn(const Value &arg);

/**
 * @brief Exponential function (EXP)
 * @param arg Exponent
 * @return e raised to the power of arg
 */
Value funcExp(const Value &arg);

/**
 * @brief Natural logarithm (LOG)
 * @param arg Value (must be positive)
 * @return Natural logarithm of arg
 * @throws RuntimeError if arg <= 0
 */
Value funcLog(const Value &arg);

/**
 * @brief Square root (SQR)
 * @param arg Value (must be non-negative)
 * @return Square root of arg
 * @throws RuntimeError if arg < 0
 */
Value funcSqr(const Value &arg);

/**
 * @brief Absolute value (ABS)
 * @param arg Value
 * @return Absolute value of arg
 */
Value funcAbs(const Value &arg);

/**
 * @brief Integer part (INT)
 * @param arg Value
 * @return Largest integer not greater than arg (floor function)
 * 
 * Examples: INT(2.7) = 2, INT(-2.7) = -3
 */
Value funcInt(const Value &arg);

/**
 * @brief Sign function (SGN)
 * @param arg Value
 * @return -1 if arg < 0, 0 if arg = 0, 1 if arg > 0
 */
Value funcSgn(const Value &arg);

/**
 * @brief Random number generator (RND)
 * @param arg Seed control: <0 reseeds, 0 repeats last, >0 generates new
 * @return Random value in range [0, 1)
 * 
 * RND(1) generates a new random number
 * RND(0) repeats the last generated number
 * RND(negative) reseeds the generator with the seed value
 */
Value funcRnd(const Value &arg);

// ============================================================================
// String Functions
// ============================================================================

/**
 * @brief String length (LEN)
 * @param arg String value
 * @return Number of characters in string
 */
Value funcLen(const Value &arg);

/**
 * @brief Convert string to number (VAL)
 * @param arg String containing numeric value
 * @return Numeric value, or 0 if string doesn't start with a number
 * 
 * Parses leading numeric portion of string. VAL("123ABC") = 123
 */
Value funcVal(const Value &arg);

/**
 * @brief ASCII code of first character (ASC)
 * @param arg Non-empty string
 * @return ASCII code of first character
 * @throws RuntimeError if string is empty
 */
Value funcAsc(const Value &arg);

/**
 * @brief Character from ASCII code (CHR$)
 * @param arg ASCII code (0-255)
 * @return Single-character string
 */
Value funcChr(const Value &arg);

/**
 * @brief Left substring (LEFT$)
 * @param str Source string
 * @param len Number of characters (from left)
 * @return Leftmost len characters of str
 */
Value funcLeft(const Value &str, const Value &len);

/**
 * @brief Right substring (RIGHT$)
 * @param str Source string
 * @param len Number of characters (from right)
 * @return Rightmost len characters of str
 */
Value funcRight(const Value &str, const Value &len);

/**
 * @brief Middle substring (MID$)
 * @param str Source string
 * @param start Starting position (1-based)
 * @param len Number of characters to extract
 * @return Substring starting at start with length len
 */
Value funcMid(const Value &str, const Value &start, const Value &len);

/**
 * @brief Convert number to string (STR$)
 * @param arg Numeric value
 * @return String representation with leading space for positive numbers
 */
Value funcStr(const Value &arg);

// ============================================================================
// Output Formatting Functions
// ============================================================================

/**
 * @brief Tab to column position (TAB)
 * @param arg Column number (1-based)
 * @return Control value for PRINT statement positioning
 * 
 * Used in PRINT statements: PRINT TAB(10);"TEXT"
 */
Value funcTab(const Value &arg);

/**
 * @brief Output spaces (SPC)
 * @param arg Number of spaces
 * @return Control value for PRINT statement spacing
 * 
 * Used in PRINT statements: PRINT SPC(5);"TEXT"
 */
Value funcSpc(const Value &arg);

/**
 * @brief Get cursor column position (POS)
 * @param arg Dummy argument (typically 0)
 * @return Current horizontal cursor position (0-based)
 * 
 * Returns current column on terminal. Implementation uses ANSI queries
 * on POSIX systems and Windows console API on Windows.
 */
Value funcPos(const Value &arg);

// ============================================================================
// System Functions
// ============================================================================

/**
 * @brief Free memory available (FRE)
 * @param arg Dummy argument (if 0) or string to trigger garbage collection
 * @return Number of bytes of free memory
 * 
 * In Applesoft, FRE(0) returns free memory. This implementation returns
 * a fixed value for compatibility.
 */
Value funcFre(const Value &arg);

/**
 * @brief Read paddle/joystick position (PDL)
 * @param arg Paddle number (0-3)
 * @return Paddle value (0-255)
 * 
 * Returns simulated paddle values for compatibility.
 */
Value funcPdl(const Value &arg);

/**
 * @brief Read memory byte (PEEK)
 * @param arg Memory address
 * @return Byte value at address (0-255)
 * @throws RuntimeError if address is outside LOMEM-HIMEM range
 * 
 * Accesses simplified memory model. Key addresses:
 * - 222 ($DE): ProDOS error code
 * - 24 ($18): Horizontal cursor position
 */
Value funcPeek(const Value &arg);

/**
 * @brief Read screen pixel color (SCRN)
 * @param x Horizontal position
 * @param y Vertical position
 * @return Color value at position
 * 
 * Returns color of pixel at specified position in current graphics mode.
 */
Value funcScrn(const Value &x, const Value &y);

/**
 * @brief Call machine language routine (USR)
 * @param addr Address of routine
 * @return Value from routine (always 0 in this implementation)
 * 
 * Simulated for compatibility; does not execute actual machine code.
 */
Value funcUsr(const Value &addr);

// ============================================================================
// Memory Operations
// ============================================================================

/**
 * @brief Write byte to memory (POKE)
 * @param addr Memory address
 * @param val Byte value (0-255)
 * @throws RuntimeError if address is outside LOMEM-HIMEM range
 * 
 * Modifies simplified memory model. Used for system configuration.
 */
void pokeMemory(int addr, int val);

/**
 * @brief Read byte from memory (PEEK)
 * @param addr Memory address
 * @return Byte value at address
 * @throws RuntimeError if address is outside LOMEM-HIMEM range
 * 
 * Helper function for PEEK(). Accesses simplified memory model.
 */
int peekMemory(int addr);

/**
 * @brief Format memory address as hexadecimal string
 * @param addr Address value
 * @param mask16bit If true, display as 16-bit address (default)
 * @return Hex string in format "0xXXXX" or "0xXXXXXXXX"
 * 
 * Used for error messages and debugging output.
 */
std::string formatHexAddress(int addr, bool mask16bit = true);

/**
 * @brief Configure memory address bounds for PEEK/POKE/WAIT
 * @param lomem Lower memory bound (inclusive)
 * @param himem Upper memory bound (inclusive)
 * 
 * Sets valid address range for memory operations. Addresses outside this
 * range trigger "MEMORY RANGE ERROR". Used to simulate different Apple II
 * memory configurations (e.g., 48K, 64K systems).
 * 
 * Default bounds match typical Applesoft configuration.
 */
void setMemoryBounds(int lomem, int himem);
