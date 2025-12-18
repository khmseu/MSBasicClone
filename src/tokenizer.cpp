#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <map>

Tokenizer::Tokenizer() : pos_(0), line_(1), column_(1) {}

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

bool Tokenizer::isKeyword(const std::string &word) const {
  static const std::map<std::string, TokenType> keywords = {
      {"PRINT", TokenType::PRINT},     {"?", TokenType::PRINT},
      {"INPUT", TokenType::INPUT},     {"LET", TokenType::LET},
      {"IF", TokenType::IF},           {"THEN", TokenType::THEN},
      {"ELSE", TokenType::ELSE},       {"GOTO", TokenType::GOTO},
      {"GOSUB", TokenType::GOSUB},     {"RETURN", TokenType::RETURN},
      {"FOR", TokenType::FOR},         {"TO", TokenType::TO},
      {"STEP", TokenType::STEP},       {"NEXT", TokenType::NEXT},
      {"DIM", TokenType::DIM},         {"DATA", TokenType::DATA},
      {"READ", TokenType::READ},       {"RESTORE", TokenType::RESTORE},
      {"REM", TokenType::REM},         {"END", TokenType::END},
      {"NEW", TokenType::NEW},         {"RUN", TokenType::RUN},
      {"LIST", TokenType::LIST},       {"LOAD", TokenType::LOAD},
      {"SAVE", TokenType::SAVE},       {"CATALOG", TokenType::CATALOG},
      {"DEF", TokenType::DEF},         {"FN", TokenType::FN},
      {"ONERR", TokenType::ONERR},     {"RESUME", TokenType::RESUME},
      {"ON", TokenType::ON},           {"CLR", TokenType::CLR},
      {"HOME", TokenType::HOME},       {"TEXT", TokenType::TEXT},
      {"GR", TokenType::GR},           {"HIRES", TokenType::HIRES},
      {"COLOR=", TokenType::COLOR},    {"HGR", TokenType::HGR},
      {"HGR2", TokenType::HGR2},       {"HCOLOR=", TokenType::HCOLOR},
      {"CALL", TokenType::CALL},       {"PEEK", TokenType::PEEK},
      {"POKE", TokenType::POKE},       {"GET", TokenType::GET},
      {"HTAB", TokenType::HTAB},       {"VTAB", TokenType::VTAB},
      {"INVERSE", TokenType::INVERSE}, {"NORMAL", TokenType::NORMAL},
      {"FLASH", TokenType::FLASH},     {"STOP", TokenType::STOP},
      {"PLOT", TokenType::PLOT},       {"HLIN", TokenType::HLIN},
      {"VLIN", TokenType::VLIN},       {"SIN", TokenType::SIN},
      {"COS", TokenType::COS},         {"TAN", TokenType::TAN},
      {"ATN", TokenType::ATN},         {"EXP", TokenType::EXP},
      {"LOG", TokenType::LOG},         {"SQR", TokenType::SQR},
      {"ABS", TokenType::ABS},         {"INT", TokenType::INT},
      {"SGN", TokenType::SGN},         {"RND", TokenType::RND},
      {"LEN", TokenType::LEN},         {"VAL", TokenType::VAL},
      {"ASC", TokenType::ASC},         {"CHR$", TokenType::CHR},
      {"LEFT$", TokenType::LEFT},      {"RIGHT$", TokenType::RIGHT},
      {"MID$", TokenType::MID},        {"STR$", TokenType::STR},
      {"TAB", TokenType::TAB},         {"SPC", TokenType::SPC},
      {"POS", TokenType::POS},         {"FRE", TokenType::FRE},
      {"PDL", TokenType::PDL},         {"AND", TokenType::AND},
      {"OR", TokenType::OR},           {"NOT", TokenType::NOT},
      {"MOD", TokenType::MOD}};

  std::string upper = word;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  return keywords.find(upper) != keywords.end();
}

TokenType Tokenizer::getKeywordType(const std::string &word) const {
  static const std::map<std::string, TokenType> keywords = {
      {"PRINT", TokenType::PRINT},     {"?", TokenType::PRINT},
      {"INPUT", TokenType::INPUT},     {"LET", TokenType::LET},
      {"IF", TokenType::IF},           {"THEN", TokenType::THEN},
      {"ELSE", TokenType::ELSE},       {"GOTO", TokenType::GOTO},
      {"GOSUB", TokenType::GOSUB},     {"RETURN", TokenType::RETURN},
      {"FOR", TokenType::FOR},         {"TO", TokenType::TO},
      {"STEP", TokenType::STEP},       {"NEXT", TokenType::NEXT},
      {"DIM", TokenType::DIM},         {"DATA", TokenType::DATA},
      {"READ", TokenType::READ},       {"RESTORE", TokenType::RESTORE},
      {"REM", TokenType::REM},         {"END", TokenType::END},
      {"NEW", TokenType::NEW},         {"RUN", TokenType::RUN},
      {"LIST", TokenType::LIST},       {"LOAD", TokenType::LOAD},
      {"SAVE", TokenType::SAVE},       {"CATALOG", TokenType::CATALOG},
      {"DEF", TokenType::DEF},         {"FN", TokenType::FN},
      {"ONERR", TokenType::ONERR},     {"RESUME", TokenType::RESUME},
      {"ON", TokenType::ON},           {"CLR", TokenType::CLR},
      {"HOME", TokenType::HOME},       {"TEXT", TokenType::TEXT},
      {"GR", TokenType::GR},           {"HIRES", TokenType::HIRES},
      {"COLOR=", TokenType::COLOR},    {"HGR", TokenType::HGR},
      {"HGR2", TokenType::HGR2},       {"HCOLOR=", TokenType::HCOLOR},
      {"CALL", TokenType::CALL},       {"PEEK", TokenType::PEEK},
      {"POKE", TokenType::POKE},       {"GET", TokenType::GET},
      {"HTAB", TokenType::HTAB},       {"VTAB", TokenType::VTAB},
      {"INVERSE", TokenType::INVERSE}, {"NORMAL", TokenType::NORMAL},
      {"FLASH", TokenType::FLASH},     {"STOP", TokenType::STOP},
      {"PLOT", TokenType::PLOT},       {"HLIN", TokenType::HLIN},
      {"VLIN", TokenType::VLIN},       {"SIN", TokenType::SIN},
      {"COS", TokenType::COS},         {"TAN", TokenType::TAN},
      {"ATN", TokenType::ATN},         {"EXP", TokenType::EXP},
      {"LOG", TokenType::LOG},         {"SQR", TokenType::SQR},
      {"ABS", TokenType::ABS},         {"INT", TokenType::INT},
      {"SGN", TokenType::SGN},         {"RND", TokenType::RND},
      {"LEN", TokenType::LEN},         {"VAL", TokenType::VAL},
      {"ASC", TokenType::ASC},         {"CHR$", TokenType::CHR},
      {"LEFT$", TokenType::LEFT},      {"RIGHT$", TokenType::RIGHT},
      {"MID$", TokenType::MID},        {"STR$", TokenType::STR},
      {"TAB", TokenType::TAB},         {"SPC", TokenType::SPC},
      {"POS", TokenType::POS},         {"FRE", TokenType::FRE},
      {"PDL", TokenType::PDL},         {"AND", TokenType::AND},
      {"OR", TokenType::OR},           {"NOT", TokenType::NOT},
      {"MOD", TokenType::MOD}};

  std::string upper = word;
  std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
  auto it = keywords.find(upper);
  return it != keywords.end() ? it->second : TokenType::IDENTIFIER;
}

void Tokenizer::skipWhitespace() {
  while (!isAtEnd() && std::isspace(peek()) && peek() != '\n') {
    advance();
  }
}

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

char Tokenizer::peek() const {
  if (isAtEnd())
    return '\0';
  return input_[pos_];
}

char Tokenizer::advance() {
  if (isAtEnd())
    return '\0';
  char ch = input_[pos_++];
  column_++;
  return ch;
}

bool Tokenizer::isAtEnd() const { return pos_ >= input_.length(); }
