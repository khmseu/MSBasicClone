/**
 * @file tokenizer.cpp
 * @brief Lexical analyzer implementation for Applesoft BASIC
 *
 * This file implements the Tokenizer class which performs lexical analysis
 * on BASIC source code, converting raw text into a stream of typed tokens.
 *
 * The tokenizer recognizes:
 * - Keywords (PRINT, IF, FOR, etc.)
 * - Operators (arithmetic, logical, relational)
 * - Literals (numbers, strings)
 * - Identifiers (variable names, function names)
 * - Delimiters (parentheses, commas, colons, semicolons)
 *
 * Special features:
 * - Case-insensitive keyword matching
 * - Support for ? as PRINT shorthand
 * - String literals with quote delimiters
 * - Multi-character operators (<=, >=, <>)
 * - Line continuation handling
 */

#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <map>

/**
 * @brief Construct a new Tokenizer
 *
 * Initializes tokenizer state with position at start of input.
 */
Tokenizer::Tokenizer() : pos_(0), line_(1), column_(1) {}

/**
 * @brief Tokenize a line of BASIC code
 *
 * Converts the input string into a vector of tokens. Handles whitespace
 * skipping and ensures no duplicate NEWLINE tokens are emitted.
 *
 * @param line Input BASIC source code line
 * @return Vector of Token objects representing the tokenized input
 */
std::vector<Token> Tokenizer::tokenize(const std::string &line) {
  input_ = line;
  pos_ = 0;
  column_ = 1;

  std::vector<Token> tokens;

  while (!isAtEnd()) {
    skipWhitespace();
    if (isAtEnd())
      break;

    Token token = nextToken();
    if (token.type != TokenType::NEWLINE || tokens.empty() ||
        tokens.back().type != TokenType::NEWLINE) {
      tokens.push_back(token);
    }
  }

  return tokens;
}

/**
 * @brief Check if a word is a BASIC keyword
 *
 * Performs case-insensitive lookup in the keyword table. This static map
 * contains all Applesoft BASIC keywords including commands, statements,
 * functions, and special symbols (like ? for PRINT).
 *
 * @param word Word to check (case-insensitive)
 * @return true if word is a keyword, false otherwise
 */
bool Tokenizer::isKeyword(const std::string &word) const {
  static const std::map<std::string, TokenType> keywords = {
      {"PRINT", TokenType::PRINT},
      {"?", TokenType::PRINT},
      {"INPUT", TokenType::INPUT},
      {"LET", TokenType::LET},
      {"IF", TokenType::IF},
      {"THEN", TokenType::THEN},
      {"ELSE", TokenType::ELSE},
      {"GOTO", TokenType::GOTO},
      {"GOSUB", TokenType::GOSUB},
      {"RETURN", TokenType::RETURN},
      {"FOR", TokenType::FOR},
      {"TO", TokenType::TO},
      {"STEP", TokenType::STEP},
      {"NEXT", TokenType::NEXT},
      {"DIM", TokenType::DIM},
      {"DATA", TokenType::DATA},
      {"READ", TokenType::READ},
      {"RESTORE", TokenType::RESTORE},
      {"REM", TokenType::REM},
      {"END", TokenType::END},
      {"NEW", TokenType::NEW},
      {"RUN", TokenType::RUN},
      {"LIST", TokenType::LIST},
      {"LOAD", TokenType::LOAD},
      {"SAVE", TokenType::SAVE},
      {"CATALOG", TokenType::CATALOG},
      {"CONT", TokenType::CONT},
      {"DEL", TokenType::DEL},
      {"DEF", TokenType::DEF},
      {"FN", TokenType::FN},
      {"ONERR", TokenType::ONERR},
      {"RESUME", TokenType::RESUME},
      {"ON", TokenType::ON},
      {"AT", TokenType::AT},
      {"CLR", TokenType::CLR},
      {"CLEAR", TokenType::CLR},
      {"HOME", TokenType::HOME},
      {"TEXT", TokenType::TEXT},
      {"GR", TokenType::GR},
      {"HIRES", TokenType::HIRES},
      {"COLOR=", TokenType::COLOR},
      {"HGR", TokenType::HGR},
      {"HGR2", TokenType::HGR2},
      {"HCOLOR=", TokenType::HCOLOR},
      {"CALL", TokenType::CALL},
      {"PEEK", TokenType::PEEK},
      {"POKE", TokenType::POKE},
      {"GET", TokenType::GET},
      {"HTAB", TokenType::HTAB},
      {"VTAB", TokenType::VTAB},
      {"INVERSE", TokenType::INVERSE},
      {"NORMAL", TokenType::NORMAL},
      {"FLASH", TokenType::FLASH},
      {"STOP", TokenType::STOP},
      {"PLOT", TokenType::PLOT},
      {"HLIN", TokenType::HLIN},
      {"VLIN", TokenType::VLIN},
      {"HPLOT", TokenType::HPLOT},
      {"XDRAW", TokenType::XDRAW},
      {"DRAW", TokenType::DRAW},
      {"MOVE", TokenType::MOVE},
      {"ROTATE", TokenType::ROTATE},
      {"SCALE", TokenType::SCALE},
      {"SHLOAD", TokenType::SHLOAD},
      {"SIN", TokenType::SIN},
      {"COS", TokenType::COS},
      {"TAN", TokenType::TAN},
      {"ATN", TokenType::ATN},
      {"EXP", TokenType::EXP},
      {"LOG", TokenType::LOG},
      {"SQR", TokenType::SQR},
      {"ABS", TokenType::ABS},
      {"INT", TokenType::INT},
      {"SGN", TokenType::SGN},
      {"RND", TokenType::RND},
      {"LEN", TokenType::LEN},
      {"VAL", TokenType::VAL},
      {"ASC", TokenType::ASC},
      {"CHR$", TokenType::CHR},
      {"LEFT$", TokenType::LEFT},
      {"RIGHT$", TokenType::RIGHT},
      {"MID$", TokenType::MID},
      {"STR$", TokenType::STR},
      {"TAB", TokenType::TAB},
      {"SPC", TokenType::SPC},
      {"POS", TokenType::POS},
      {"FRE", TokenType::FRE},
      {"PDL", TokenType::PDL},
      {"AND", TokenType::AND},
      {"OR", TokenType::OR},
      {"NOT", TokenType::NOT},
      {"MOD", TokenType::MOD},
      {"TRACE", TokenType::TRACE},
      {"NOTRACE", TokenType::NOTRACE},
      {"RANDOMIZE", TokenType::RANDOMIZE},
      {"SPEED", TokenType::SPEED},
      {"PR", TokenType::PR},
      {"IN", TokenType::IN},
      {"WHILE", TokenType::WHILE},
      {"WEND", TokenType::WEND},
      {"POP", TokenType::POP},
      {"WAIT", TokenType::WAIT},
      {"HIMEM", TokenType::HIMEM},
      {"LOMEM", TokenType::LOMEM},
      {"SCRN", TokenType::SCRN},
      {"RECALL", TokenType::RECALL},
      {"STORE", TokenType::STORE},
      {"TAPE", TokenType::TAPE},
      {"DELETE", TokenType::DELETE},
      {"RENAME", TokenType::RENAME},
      {"PREFIX", TokenType::PREFIX},
      {"OPEN", TokenType::OPEN},
      {"CLOSE", TokenType::CLOSE},
      {"APPEND", TokenType::APPEND},
      {"BLOAD", TokenType::BLOAD},
      {"BRUN", TokenType::BRUN},
      {"BSAVE", TokenType::BSAVE},
      {"CREATE", TokenType::CREATE},
      {"FLUSH", TokenType::FLUSH},
      {"LOCK", TokenType::LOCK},
      {"UNLOCK", TokenType::UNLOCK},
      {"POSITION", TokenType::POSITION},
      {"CHAIN", TokenType::CHAIN},
      {"EXEC", TokenType::EXEC},
      {"CAT", TokenType::CAT},
      {"WRITE", TokenType::PRODOSWRITE},
      {"-", TokenType::DASH},
      {"USR", TokenType::USR}};

  std::string upper = word;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return keywords.find(upper) != keywords.end();
}

/**
 * @brief Resolve a keyword string to its TokenType
 *
 * Performs case-insensitive lookup in the tokenizer's keyword table and
 * returns the corresponding TokenType.
 *
 * Details:
 * - The keyword map includes commands (PRINT, INPUT, LIST, RUN, NEW),
 *   control flow (IF, THEN, ELSE, FOR, NEXT, GOTO, GOSUB), data keywords
 *   (DATA, READ, RESTORE, DIM), functions (FN, DEF), graphics (GR, HGR,
 *   HPLOT, DRAW, XDRAW), and ProDOS commands (OPEN, CLOSE, READ, WRITE).
 * - Special tokens like "?" (alias for PRINT) and multi-character tokens
 *   such as "COLOR=" and "HCOLOR=" are included.
 * - Keywords are matched case-insensitively; input is converted to uppercase.
 *
 * Notes:
 * - If the word is not recognized as a keyword, returns TokenType::IDENTIFIER.
 *
 * @param word Input word to classify (case-insensitive)
 * @return TokenType for the keyword, or TokenType::IDENTIFIER if not found
 */
TokenType Tokenizer::getKeywordType(const std::string &word) const {
  static const std::map<std::string, TokenType> keywords = {
      {"PRINT", TokenType::PRINT},
      {"?", TokenType::PRINT},
      {"INPUT", TokenType::INPUT},
      {"LET", TokenType::LET},
      {"IF", TokenType::IF},
      {"THEN", TokenType::THEN},
      {"ELSE", TokenType::ELSE},
      {"GOTO", TokenType::GOTO},
      {"GOSUB", TokenType::GOSUB},
      {"RETURN", TokenType::RETURN},
      {"FOR", TokenType::FOR},
      {"TO", TokenType::TO},
      {"STEP", TokenType::STEP},
      {"NEXT", TokenType::NEXT},
      {"DIM", TokenType::DIM},
      {"DATA", TokenType::DATA},
      {"READ", TokenType::READ},
      {"RESTORE", TokenType::RESTORE},
      {"REM", TokenType::REM},
      {"END", TokenType::END},
      {"NEW", TokenType::NEW},
      {"RUN", TokenType::RUN},
      {"LIST", TokenType::LIST},
      {"LOAD", TokenType::LOAD},
      {"SAVE", TokenType::SAVE},
      {"CATALOG", TokenType::CATALOG},
      {"CONT", TokenType::CONT},
      {"DEL", TokenType::DEL},
      {"DEF", TokenType::DEF},
      {"FN", TokenType::FN},
      {"ONERR", TokenType::ONERR},
      {"RESUME", TokenType::RESUME},
      {"ON", TokenType::ON},
      {"AT", TokenType::AT},
      {"CLR", TokenType::CLR},
      {"CLEAR", TokenType::CLR},
      {"HOME", TokenType::HOME},
      {"TEXT", TokenType::TEXT},
      {"GR", TokenType::GR},
      {"HIRES", TokenType::HIRES},
      {"COLOR=", TokenType::COLOR},
      {"HGR", TokenType::HGR},
      {"HGR2", TokenType::HGR2},
      {"HCOLOR=", TokenType::HCOLOR},
      {"CALL", TokenType::CALL},
      {"PEEK", TokenType::PEEK},
      {"POKE", TokenType::POKE},
      {"GET", TokenType::GET},
      {"HTAB", TokenType::HTAB},
      {"VTAB", TokenType::VTAB},
      {"INVERSE", TokenType::INVERSE},
      {"NORMAL", TokenType::NORMAL},
      {"FLASH", TokenType::FLASH},
      {"STOP", TokenType::STOP},
      {"PLOT", TokenType::PLOT},
      {"HLIN", TokenType::HLIN},
      {"VLIN", TokenType::VLIN},
      {"HPLOT", TokenType::HPLOT},
      {"XDRAW", TokenType::XDRAW},
      {"DRAW", TokenType::DRAW},
      {"MOVE", TokenType::MOVE},
      {"ROTATE", TokenType::ROTATE},
      {"SCALE", TokenType::SCALE},
      {"SHLOAD", TokenType::SHLOAD},
      {"SIN", TokenType::SIN},
      {"COS", TokenType::COS},
      {"TAN", TokenType::TAN},
      {"ATN", TokenType::ATN},
      {"EXP", TokenType::EXP},
      {"LOG", TokenType::LOG},
      {"SQR", TokenType::SQR},
      {"ABS", TokenType::ABS},
      {"INT", TokenType::INT},
      {"SGN", TokenType::SGN},
      {"RND", TokenType::RND},
      {"LEN", TokenType::LEN},
      {"VAL", TokenType::VAL},
      {"ASC", TokenType::ASC},
      {"CHR$", TokenType::CHR},
      {"LEFT$", TokenType::LEFT},
      {"RIGHT$", TokenType::RIGHT},
      {"MID$", TokenType::MID},
      {"STR$", TokenType::STR},
      {"TAB", TokenType::TAB},
      {"SPC", TokenType::SPC},
      {"POS", TokenType::POS},
      {"FRE", TokenType::FRE},
      {"PDL", TokenType::PDL},
      {"AND", TokenType::AND},
      {"OR", TokenType::OR},
      {"NOT", TokenType::NOT},
      {"MOD", TokenType::MOD},
      {"TRACE", TokenType::TRACE},
      {"NOTRACE", TokenType::NOTRACE},
      {"RANDOMIZE", TokenType::RANDOMIZE},
      {"SPEED", TokenType::SPEED},
      {"PR", TokenType::PR},
      {"IN", TokenType::IN},
      {"WHILE", TokenType::WHILE},
      {"WEND", TokenType::WEND},
      {"POP", TokenType::POP},
      {"WAIT", TokenType::WAIT},
      {"HIMEM", TokenType::HIMEM},
      {"LOMEM", TokenType::LOMEM},
      {"SCRN", TokenType::SCRN},
      {"RECALL", TokenType::RECALL},
      {"STORE", TokenType::STORE},
      {"TAPE", TokenType::TAPE},
      {"DELETE", TokenType::DELETE},
      {"RENAME", TokenType::RENAME},
      {"PREFIX", TokenType::PREFIX},
      {"OPEN", TokenType::OPEN},
      {"CLOSE", TokenType::CLOSE},
      {"APPEND", TokenType::APPEND},
      {"BLOAD", TokenType::BLOAD},
      {"BRUN", TokenType::BRUN},
      {"BSAVE", TokenType::BSAVE},
      {"CREATE", TokenType::CREATE},
      {"FLUSH", TokenType::FLUSH},
      {"LOCK", TokenType::LOCK},
      {"UNLOCK", TokenType::UNLOCK},
      {"POSITION", TokenType::POSITION},
      {"CHAIN", TokenType::CHAIN},
      {"EXEC", TokenType::EXEC},
      {"CAT", TokenType::CAT},
      {"WRITE", TokenType::PRODOSWRITE},
      {"-", TokenType::DASH},
      {"USR", TokenType::USR}};

  std::string upper = word;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  auto it = keywords.find(upper);
  return it != keywords.end() ? it->second : TokenType::IDENTIFIER;
}

/**
 * @brief Skip whitespace characters
 *
 * Advances the tokenizer position past spaces and tabs, but stops at newlines.
 * This preserves newline tokens which are significant in BASIC syntax.
 */
void Tokenizer::skipWhitespace() {
  while (!isAtEnd() && std::isspace(peek()) && peek() != '\n') {
    advance();
  }
}

/**
 * @brief Read next token from input
 *
 * Dispatches to specific token readers based on the first character:
 * - Digits or decimal point: readNumber()
 * - Double quote: readString()
 * - Letter: readIdentifier() (keywords or variable names)
 * - Other: readOperator() (operators, delimiters, special characters)
 *
 * @return Next token from input stream
 */
Token Tokenizer::nextToken() {
  char ch = peek();

  // Numbers
  if (std::isdigit(ch) || (ch == '.' && pos_ + 1 < input_.length() &&
                           std::isdigit(input_[pos_ + 1]))) {
    return readNumber();
  }

  // Strings
  if (ch == '"') {
    return readString();
  }

  // Identifiers and keywords
  if (std::isalpha(ch)) {
    return readIdentifier();
  }

  // Operators and delimiters
  return readOperator();
}

/**
 * @brief Read a numeric literal token
 *
 * Parses numeric literals including:
 * - Integers: 123, 42
 * - Decimals: 3.14, .5
 * - Scientific notation: 1.23E5, 6.022e-23
 *
 * The parser handles:
 * - Optional decimal point (at most one)
 * - Optional exponent (E or e) with optional sign
 * - No leading spaces (caller has already positioned at first digit)
 *
 * @return NUMBER token with parsed value
 */
Token Tokenizer::readNumber() {
  Token token;
  token.type = TokenType::NUMBER;
  token.line = line_;
  token.column = column_;

  std::string numStr;
  bool hasDecimal = false;
  bool hasExponent = false;

  while (!isAtEnd()) {
    char ch = peek();

    if (std::isdigit(ch)) {
      numStr += advance();
    } else if (ch == '.' && !hasDecimal && !hasExponent) {
      hasDecimal = true;
      numStr += advance();
    } else if ((ch == 'E' || ch == 'e') && !hasExponent) {
      hasExponent = true;
      numStr += advance();
      if (!isAtEnd() && (peek() == '+' || peek() == '-')) {
        numStr += advance();
      }
    } else {
      break;
    }
  }

  token.text = numStr;
  token.value = Value(std::stod(numStr));
  return token;
}

/**
 * @brief Read a string literal token
 *
 * Parses string literals enclosed in double quotes.
 * String literals can contain any characters except:
 * - Closing double quote (ends the string)
 * - Newline (strings cannot span lines in Applesoft BASIC)
 *
 * Examples:
 *   "HELLO"
 *   "Hello, World!"
 *   ""  (empty string)
 *
 * @return STRING token with the string content (quotes not included)
 */
Token Tokenizer::readString() {
  Token token;
  token.type = TokenType::STRING;
  token.line = line_;
  token.column = column_;

  advance(); // Skip opening quote

  std::string str;
  while (!isAtEnd() && peek() != '"' && peek() != '\n') {
    str += advance();
  }

  if (!isAtEnd() && peek() == '"') {
    advance(); // Skip closing quote
  }

  token.text = str;
  token.value = Value(str);
  return token;
}

/**
 * @brief Read an identifier or keyword token
 *
 * Parses identifiers (variable names, function names) and keywords.
 * Identifiers can contain:
 * - Letters (A-Z, case insensitive)
 * - Digits (but not as first character)
 * - $ suffix for string variables
 * - % suffix for integer variables
 *
 * Special handling:
 * - Keywords are case-insensitive and converted to uppercase
 * - Built-in string functions (CHR$, LEFT$, etc.) are recognized as keywords
 * - FN prefix indicates user-defined function call
 *
 * Examples:
 *   PRINT → PRINT keyword
 *   X → variable identifier
 *   NAME$ → string variable identifier
 *   COUNT% → integer variable identifier
 *   FNXY → user function call
 *
 * @return Keyword token or IDENTIFIER token
 */
Token Tokenizer::readIdentifier() {
  Token token;
  token.line = line_;
  token.column = column_;

  std::string ident;
  while (!isAtEnd() &&
         (std::isalnum(peek()) || peek() == '$' || peek() == '%')) {
    ident += advance();
  }

  // Check for function names with $
  if (!ident.empty() && ident.back() == '$') {
    std::string funcName = ident;
    std::transform(funcName.begin(), funcName.end(), funcName.begin(),
                   ::toupper);

    if (funcName == "CHR$" || funcName == "LEFT$" || funcName == "RIGHT$" ||
        funcName == "MID$" || funcName == "STR$") {
      token.type = getKeywordType(funcName);
      token.text = funcName;
      return token;
    }
  }

  token.text = ident;

  if (isKeyword(ident)) {
    token.type = getKeywordType(ident);
  } else {
    token.type = TokenType::IDENTIFIER;
  }

  return token;
}

/**
 * @brief Read an operator or delimiter token
 *
 * Parses operators, delimiters, and special characters including:
 * - Arithmetic: +, -, *, /, ^, MOD
 * - Relational: =, <>, <, >, <=, >=
 * - Logical: AND, OR, NOT
 * - Delimiters: (, ), ,, ;, :
 * - Special: ? (shorthand for PRINT)
 *
 * Multi-character operators:
 * - <= (less than or equal)
 * - >= (greater than or equal)
 * - <> (not equal)
 *
 * The tokenizer looks ahead one character to identify multi-character
 * operators before falling back to single-character tokens.
 *
 * @return Operator or delimiter token
 */
Token Tokenizer::readOperator() {
  Token token;
  token.line = line_;
  token.column = column_;

  char ch = peek();

  switch (ch) {
  case '+':
    token.type = TokenType::PLUS;
    token.text = "+";
    advance();
    break;
  case '-':
    token.type = TokenType::MINUS;
    token.text = "-";
    advance();
    break;
  case '*':
    token.type = TokenType::MULTIPLY;
    token.text = "*";
    advance();
    break;
  case '/':
    token.type = TokenType::DIVIDE;
    token.text = "/";
    advance();
    break;
  case '^':
    token.type = TokenType::POWER;
    token.text = "^";
    advance();
    break;
  case '(':
    token.type = TokenType::LPAREN;
    token.text = "(";
    advance();
    break;
  case ')':
    token.type = TokenType::RPAREN;
    token.text = ")";
    advance();
    break;
  case ',':
    token.type = TokenType::COMMA;
    token.text = ",";
    advance();
    break;
  case ';':
    token.type = TokenType::SEMICOLON;
    token.text = ";";
    advance();
    break;
  case ':':
    token.type = TokenType::COLON;
    token.text = ":";
    advance();
    break;
  case '#':
    token.type = TokenType::HASH;
    token.text = "#";
    advance();
    break;
  case '$':
    token.type = TokenType::DOLLAR;
    token.text = "$";
    advance();
    break;
  case '%':
    token.type = TokenType::PERCENT;
    token.text = "%";
    advance();
    break;
  case '\n':
    token.type = TokenType::NEWLINE;
    token.text = "\\n";
    advance();
    break;

  case '=':
    token.text = "=";
    advance();
    token.type = TokenType::EQUAL;
    break;

  case '<':
    token.text = "<";
    advance();
    if (!isAtEnd()) {
      if (peek() == '=') {
        token.text += "=";
        token.type = TokenType::LESS_EQUAL;
        advance();
      } else if (peek() == '>') {
        token.text += ">";
        token.type = TokenType::NOT_EQUAL;
        advance();
      } else {
        token.type = TokenType::LESS;
      }
    } else {
      token.type = TokenType::LESS;
    }
    break;

  case '>':
    token.text = ">";
    advance();
    if (!isAtEnd() && peek() == '=') {
      token.text += "=";
      token.type = TokenType::GREATER_EQUAL;
      advance();
    } else {
      token.type = TokenType::GREATER;
    }
    break;

  default:
    // Unknown character, skip it
    token.type = TokenType::NEWLINE;
    token.text = std::string(1, ch);
    advance();
    break;
  }

  return token;
}

/**
 * @brief Peek at current character without advancing
 *
 * Returns the character at the current position without consuming it.
 * Used for lookahead in tokenization.
 *
 * @return Current character, or '\0' if at end of input
 */
char Tokenizer::peek() const {
  if (isAtEnd())
    return '\0';
  return input_[pos_];
}

/**
 * @brief Consume and return current character
 *
 * Advances the position and column counters, returning the character
 * that was consumed. This is the primary method for consuming input
 * during tokenization.
 *
 * @return Character at current position, or '\0' if at end
 */
char Tokenizer::advance() {
  if (isAtEnd())
    return '\0';
  char ch = input_[pos_++];
  column_++;
  return ch;
}

/**
 * @brief Check if tokenizer is at end of input (tokenizer helper)
 * 
 * Returns true if the current position has reached or exceeded the length
 * of the input string. Used throughout tokenization to prevent reading
 * past the end of input.
 * 
 * @return true if at end of input, false otherwise
 */
bool Tokenizer::isAtEnd() const { return pos_ >= input_.length(); }
