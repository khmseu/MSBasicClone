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

// Line number type
using LineNumber = int;

// Token types
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

// Value type - represents either a number or string
class Value {
public:
  Value();
  explicit Value(double num);
  explicit Value(const std::string &str);
  explicit Value(const Float40 &f40);

  bool isNumber() const;
  bool isString() const;

  double getNumber() const;
  std::string getString() const;

  Value operator+(const Value &other) const;
  Value operator-(const Value &other) const;
  Value operator*(const Value &other) const;
  Value operator/(const Value &other) const;

  bool operator==(const Value &other) const;
  bool operator!=(const Value &other) const;
  bool operator<(const Value &other) const;
  bool operator>(const Value &other) const;
  bool operator<=(const Value &other) const;
  bool operator>=(const Value &other) const;

private:
  std::variant<double, std::string> data;
};

// Token structure
struct Token {
  TokenType type;
  std::string text;
  Value value;
  int line;
  int column;
};

// Program line structure
struct ProgramLine {
  LineNumber lineNumber;
  std::string text;
  std::vector<Token> tokens;
  std::vector<std::shared_ptr<Statement>> statements;
};
