#pragma once

#include "tokenizer.h"
#include "types.h"
#include "variables.h"
#include <memory>
#include <string>
#include <vector>

// Expression base class
class Expression {
public:
  virtual ~Expression() = default;
  virtual Value evaluate(class Interpreter *interp) = 0;
};

// Statement base class
class Statement {
public:
  virtual ~Statement() = default;
  virtual void execute(class Interpreter *interp) = 0;

  // Allows statements like DATA to contribute to pre-run collection.
  virtual void collectData(std::vector<Value> &) const {}
};

class Parser {
public:
  Parser();

  // Parse a line of tokens into statements
  std::vector<std::shared_ptr<Statement>>
  parse(const std::vector<Token> &tokens);

  // Parse an expression from tokens
  std::shared_ptr<Expression> parseExpression(const std::vector<Token> &tokens,
                                              size_t &pos);

private:
  std::shared_ptr<Expression>
  parseOrExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parseAndExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parseNotExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parseRelationalExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parseAdditiveExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parseMultiplicativeExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parseUnaryExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parsePowerExpression(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Expression>
  parsePrimaryExpression(const std::vector<Token> &tokens, size_t &pos);

  std::shared_ptr<Statement> parseStatement(const std::vector<Token> &tokens,
                                            size_t &pos);
  std::shared_ptr<Statement> parsePrint(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement> parseInput(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement>
  parseLetOrAssignment(const std::vector<Token> &tokens, size_t &pos);
  std::shared_ptr<Statement> parseIf(const std::vector<Token> &tokens,
                                     size_t &pos);
  std::shared_ptr<Statement> parseGoto(const std::vector<Token> &tokens,
                                       size_t &pos);
  std::shared_ptr<Statement> parseGosub(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement> parseFor(const std::vector<Token> &tokens,
                                      size_t &pos);
  std::shared_ptr<Statement> parseNext(const std::vector<Token> &tokens,
                                       size_t &pos);
  std::shared_ptr<Statement> parseDim(const std::vector<Token> &tokens,
                                      size_t &pos);
  std::shared_ptr<Statement> parseData(const std::vector<Token> &tokens,
                                       size_t &pos);
  std::shared_ptr<Statement> parseRead(const std::vector<Token> &tokens,
                                       size_t &pos);
  std::shared_ptr<Statement> parseDef(const std::vector<Token> &tokens,
                                      size_t &pos);
  std::shared_ptr<Statement> parseOnErr(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement> parseOn(const std::vector<Token> &tokens,
                                     size_t &pos);
  std::shared_ptr<Statement> parseRandomize(const std::vector<Token> &tokens,
                                            size_t &pos);
  std::shared_ptr<Statement> parseSpeed(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement>
  parseDeviceRedirect(const std::vector<Token> &tokens, size_t &pos,
                      bool isOutput);
  std::shared_ptr<Statement> parseWhile(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement> parseWait(const std::vector<Token> &tokens,
                                       size_t &pos);
  std::shared_ptr<Statement> parseHimem(const std::vector<Token> &tokens,
                                        size_t &pos);
  std::shared_ptr<Statement> parseLomem(const std::vector<Token> &tokens,
                                        size_t &pos);

  bool match(const std::vector<Token> &tokens, size_t pos,
             TokenType type) const;
  bool isAtEnd(const std::vector<Token> &tokens, size_t pos) const;
};
