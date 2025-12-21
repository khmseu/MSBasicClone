/**
 * @file types.h
 * @brief Core type definitions for the MSBasic interpreter
 * 
 * This file defines the fundamental types used throughout the MSBasic interpreter,
 * including token types, values, program structures, and AST node types.
 * It serves as the central type system for the entire interpreter.
 */

#pragma once

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>

// Forward declarations
class Float40;
class Value;
class Expression;
class Statement;
class Program;
class Variables;
class Interpreter;

/**
 * @brief Type alias for BASIC line numbers
 * 
 * Line numbers in Applesoft BASIC range from 0 to 32767.
 */
using LineNumber = int;

/**
 * @brief Enumeration of all token types recognized by the tokenizer
 * 
 * This enum defines all possible token types that can appear in BASIC code,
 * including literals, keywords, operators, built-in functions, and delimiters.
 * The tokenizer converts raw text into a stream of these typed tokens.
 */
enum class TokenType {
  // Literals
  NUMBER,
  STRING,

  // Identifiers
  IDENTIFIER,

  // Keywords
  PRINT,
  INPUT,
  LET,
  IF,
  THEN,
  ELSE,
  GOTO,
  GOSUB,
  RETURN,
  FOR,
  TO,
  STEP,
  NEXT,
  DIM,
  DATA,
  READ,
  RESTORE,
  REM,
  END,
  NEW,
  RUN,
  LIST,
  LOAD,
  SAVE,
  CATALOG,
  CONT,
  DEL,
  DEF,
  FN,
  ONERR,
  RESUME,
  CLR,
  HOME,
  TEXT,
  GR,
  HIRES,
  CALL,
  PEEK,
  POKE,
  GET,
  HTAB,
  VTAB,
  INVERSE,
  NORMAL,
  FLASH,
  HGR,
  HGR2,
  HCOLOR,
  COLOR,

  // Additional statements
  STOP,
  ON,
  AT,
  TRACE,
  NOTRACE,
  RANDOMIZE,
  SPEED,
  PR,
  IN,
  WHILE,
  WEND,
  POP,
  WAIT,
  HIMEM,
  LOMEM,
  SCRN,
  USR,
  RECALL,
  STORE,
  TAPE,

  // ProDOS commands
  DELETE,
  RENAME,
  PREFIX,
  OPEN,
  CLOSE,
  APPEND,
  BLOAD,
  BRUN,
  BSAVE,
  CREATE,
  FLUSH,
  LOCK,
  UNLOCK,
  POSITION,
  CHAIN,
  EXEC,
  DASH,
  CAT,
  PRODOSREAD,    // ProDOS READ (to avoid conflict with DATA READ)
  PRODOSWRITE,   // ProDOS WRITE
  PRODOSRESTORE, // ProDOS RESTORE (to avoid conflict with DATA RESTORE)
  PRODOSSTORE,   // ProDOS STORE (to avoid conflict with array STORE)

  // Graphics primitives
  PLOT,
  HLIN,
  VLIN,
  HPLOT,
  XDRAW,
  DRAW,
  MOVE,
  ROTATE,
  SCALE,
  SHLOAD,

  // Operators
  PLUS,
  MINUS,
  MULTIPLY,
  DIVIDE,
  POWER,
  MOD,
  EQUAL,
  NOT_EQUAL,
  LESS,
  GREATER,
  LESS_EQUAL,
  GREATER_EQUAL,
  AND,
  OR,
  NOT,

  // Delimiters
  LPAREN,
  RPAREN,
  COMMA,
  SEMICOLON,
  COLON,
  HASH,
  DOLLAR,
  PERCENT,

  // Special
  NEWLINE,
  END_OF_FILE,

  // Built-in functions
  SIN,
  COS,
  TAN,
  ATN,
  EXP,
  LOG,
  SQR,
  ABS,
  INT,
  SGN,
  RND,
  LEN,
  VAL,
  ASC,
  CHR,
  LEFT,
  RIGHT,
  MID,
  STR,
  TAB,
  SPC,
  POS,
  FRE,
  PDL
};

/**
 * @class Value
 * @brief Runtime value that can hold either a number or a string
 * 
 * Value is the fundamental data type for all runtime computations in MSBasic.
 * It uses std::variant to store either a double (for numeric values) or
 * std::string (for string values). The class provides type-safe access
 * and automatic type checking for operations.
 * 
 * Type coercion rules:
 * - Numeric operations on strings attempt conversion via VAL()
 * - String concatenation converts numbers to strings via STR$()
 * - Comparisons work within types; cross-type comparisons convert to string
 */
class Value {
public:
  /** @brief Default constructor creates a numeric value of 0.0 */
  Value();
  
  /**
   * @brief Construct a numeric value
   * @param num The numeric value to store
   */
  explicit Value(double num);
  
  /**
   * @brief Construct a string value
   * @param str The string value to store
   */
  explicit Value(const std::string &str);
  
  /**
   * @brief Construct from a 40-bit floating-point value
   * @param f40 The Float40 value to convert and store
   */
  explicit Value(const Float40 &f40);

  /**
   * @brief Check if this value is a number
   * @return true if the value holds a number, false if it holds a string
   */
  bool isNumber() const;
  
  /**
   * @brief Check if this value is a string
   * @return true if the value holds a string, false if it holds a number
   */
  bool isString() const;

  /**
   * @brief Get the numeric value
   * @return The numeric value as a double
   * @throws std::runtime_error if the value is not a number
   */
  double getNumber() const;
  
  /**
   * @brief Get the string value
   * @return The string value
   * @throws std::runtime_error if the value is not a string
   */
  std::string getString() const;

  /**
   * @brief Add two values (or concatenate strings)
   * @param other The value to add
   * @return The result of the addition/concatenation
   * @throws std::runtime_error on type mismatch
   */
  Value operator+(const Value &other) const;
  
  /**
   * @brief Subtract two numeric values
   * @param other The value to subtract
   * @return The result of the subtraction
   * @throws std::runtime_error if either value is not a number
   */
  Value operator-(const Value &other) const;
  
  /**
   * @brief Multiply two numeric values
   * @param other The value to multiply by
   * @return The result of the multiplication
   * @throws std::runtime_error if either value is not a number
   */
  Value operator*(const Value &other) const;
  
  /**
   * @brief Divide two numeric values
   * @param other The divisor
   * @return The result of the division
   * @throws std::runtime_error if either value is not a number or divisor is zero
   */
  Value operator/(const Value &other) const;

  /**
   * @brief Compare two values for equality
   * @param other The value to compare with
   * @return true if values are equal (same type and same value)
   */
  bool operator==(const Value &other) const;
  
  /**
   * @brief Compare two values for inequality
   * @param other The value to compare with
   * @return true if values are not equal
   */
  bool operator!=(const Value &other) const;
  
  /**
   * @brief Less-than comparison
   * @param other The value to compare with
   * @return true if this value is less than other
   * @throws std::runtime_error on type mismatch
   */
  bool operator<(const Value &other) const;
  
  /**
   * @brief Greater-than comparison
   * @param other The value to compare with
   * @return true if this value is greater than other
   * @throws std::runtime_error on type mismatch
   */
  bool operator>(const Value &other) const;
  
  /**
   * @brief Less-than-or-equal comparison
   * @param other The value to compare with
   * @return true if this value is less than or equal to other
   * @throws std::runtime_error on type mismatch
   */
  bool operator<=(const Value &other) const;
  
  /**
   * @brief Greater-than-or-equal comparison
   * @param other The value to compare with
   * @return true if this value is greater than or equal to other
   * @throws std::runtime_error on type mismatch
   */
  bool operator>=(const Value &other) const;

private:
  /** @brief Internal storage for either double or string */
  std::variant<double, std::string> data;
};

/**
 * @struct Token
 * @brief Represents a single lexical token from the source code
 * 
 * Tokens are the output of the tokenizer and input to the parser.
 * Each token has a type, original text, optional value (for literals),
 * and position information for error reporting.
 */
struct Token {
  /** @brief The type of this token */
  TokenType type;
  /** @brief Original text from source code */
  std::string text;
  /** @brief The value associated with this token (for literals) */
  Value value;
  /** @brief Source line number where this token appears */
  int line;
  /** @brief Column position in the source line */
  int column;
};

/**
 * @struct ProgramLine
 * @brief Represents a single numbered line in a BASIC program
 * 
 * Each program line contains its line number, original source text,
 * tokenized representation, and parsed statement ASTs ready for execution.
 */
struct ProgramLine {
  /** @brief The line number (0-32767) */
  LineNumber lineNumber;
  /** @brief Original source text of the line */
  std::string text;
  /** @brief Tokenized representation of the line */
  std::vector<Token> tokens;
  /** @brief Parsed statements ready for execution */
  std::vector<std::shared_ptr<Statement>> statements;
};
