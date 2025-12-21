/**
 * @file parser.h
 * @brief Syntax analyzer (parser) that builds AST from tokens
 * 
 * The parser implements a recursive descent parser for Applesoft BASIC syntax.
 * It converts the token stream from the tokenizer into an Abstract Syntax Tree
 * (AST) composed of Expression and Statement nodes that can be executed by
 * the interpreter.
 * 
 * Parser features:
 * - Recursive descent parsing with proper operator precedence
 * - Expression parsing (arithmetic, logical, relational, string)
 * - Statement parsing (all BASIC statements)
 * - Multi-statement line support (colon separator)
 * - Error detection and reporting
 * 
 * Operator precedence (lowest to highest):
 * 1. OR
 * 2. AND
 * 3. NOT
 * 4. Relational (=, <>, <, >, <=, >=)
 * 5. Additive (+, -)
 * 6. Multiplicative (*, /, MOD)
 * 7. Unary (+, -, NOT)
 * 8. Power (^)
 * 9. Primary (literals, variables, functions, parentheses)
 */

#pragma once

#include "tokenizer.h"
#include "types.h"
#include "variables.h"
#include <memory>
#include <string>
#include <vector>

/**
 * @class Expression
 * @brief Base class for all expression AST nodes
 * 
 * Expression is an abstract base class representing any expression that
 * can be evaluated to produce a Value. Concrete subclasses implement
 * specific expression types (literals, variables, binary operations, etc.).
 * 
 * All expressions must implement the evaluate() method which computes
 * the expression's value given an Interpreter context.
 */
class Expression {
public:
  virtual ~Expression() = default;
  
  /**
   * @brief Evaluate this expression to produce a value
   * 
   * @param interp Pointer to the interpreter (for variable lookup, etc.)
   * @return Value The computed value of the expression
   * @throws std::runtime_error On evaluation errors (type mismatch, etc.)
   */
  virtual Value evaluate(class Interpreter *interp) = 0;
};

/**
 * @class Statement
 * @brief Base class for all statement AST nodes
 * 
 * Statement is an abstract base class representing any executable statement
 * in a BASIC program. Concrete subclasses implement specific statement types
 * (PRINT, IF, FOR, etc.).
 * 
 * All statements must implement the execute() method which performs the
 * statement's action in the context of an Interpreter.
 */
class Statement {
public:
  virtual ~Statement() = default;
  
  /**
   * @brief Execute this statement
   * 
   * @param interp Pointer to the interpreter (for I/O, control flow, etc.)
   * @throws std::runtime_error On execution errors
   */
  virtual void execute(class Interpreter *interp) = 0;

  /**
   * @brief Allow statements like DATA to contribute to pre-run data collection
   * 
   * Called during program initialization to gather all DATA values before
   * execution begins. Default implementation does nothing; overridden by
   * DataStatement.
   * 
   * @param dataValues Vector to append data values to
   */
  virtual void collectData(std::vector<Value> &dataValues) const {}
};

/**
 * @class Parser
 * @brief Recursive descent parser for BASIC syntax
 * 
 * The Parser class converts a stream of tokens into an Abstract Syntax Tree
 * (AST) that can be executed by the Interpreter. It implements recursive
 * descent parsing with proper operator precedence and handles all Applesoft
 * BASIC syntax.
 * 
 * Usage:
 * @code
 * Parser parser;
 * std::vector<Token> tokens = tokenizer.tokenize(line);
 * auto statements = parser.parse(tokens);
 * for (auto& stmt : statements) {
 *     stmt->execute(interpreter);
 * }
 * @endcode
 * 
 * The parser maintains position in the token stream and provides error
 * messages with token location information.
 */
class Parser {
public:
  /**
   * @brief Construct a new Parser
   */
  Parser();

  /**
   * @brief Parse a line of tokens into statements
   * 
   * Converts a token sequence into a vector of statement AST nodes.
   * Supports multiple statements per line (colon-separated).
   * 
   * @param tokens The token sequence to parse
   * @return std::vector<std::shared_ptr<Statement>> The parsed statements
   * @throws std::runtime_error On syntax errors
   */
  std::vector<std::shared_ptr<Statement>>
  parse(const std::vector<Token> &tokens);

  /**
   * @brief Parse an expression from tokens
   * 
   * Entry point for expression parsing. Parses a complete expression
   * starting at the given position.
   * 
   * @param tokens The token sequence
   * @param pos Current position in tokens (updated on return)
   * @return std::shared_ptr<Expression> The parsed expression AST
   * @throws std::runtime_error On syntax errors
   */
  std::shared_ptr<Expression> parseExpression(const std::vector<Token> &tokens,
                                              size_t &pos);

private:
  // Expression parsing methods (in order of precedence, lowest to highest)
  
  /** @brief Parse OR expression (lowest precedence) */
  std::shared_ptr<Expression>
  parseOrExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse AND expression */
  std::shared_ptr<Expression>
  parseAndExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse NOT expression */
  std::shared_ptr<Expression>
  parseNotExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse relational expression (=, <>, <, >, <=, >=) */
  std::shared_ptr<Expression>
  parseRelationalExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse additive expression (+, -) */
  std::shared_ptr<Expression>
  parseAdditiveExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse multiplicative expression (*, /, MOD) */
  std::shared_ptr<Expression>
  parseMultiplicativeExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse unary expression (+, -, NOT) */
  std::shared_ptr<Expression>
  parseUnaryExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse power expression (^) */
  std::shared_ptr<Expression>
  parsePowerExpression(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse primary expression (literals, variables, functions, parentheses) */
  std::shared_ptr<Expression>
  parsePrimaryExpression(const std::vector<Token> &tokens, size_t &pos);

  // Statement parsing methods
  
  /** @brief Parse any statement based on leading keyword */
  std::shared_ptr<Statement> parseStatement(const std::vector<Token> &tokens,
                                            size_t &pos);
  
  /** @brief Parse PRINT statement */
  std::shared_ptr<Statement> parsePrint(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse INPUT statement */
  std::shared_ptr<Statement> parseInput(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse LET or implicit assignment statement */
  std::shared_ptr<Statement>
  parseLetOrAssignment(const std::vector<Token> &tokens, size_t &pos);
  
  /** @brief Parse IF...THEN...ELSE statement */
  std::shared_ptr<Statement> parseIf(const std::vector<Token> &tokens,
                                     size_t &pos);
  
  /** @brief Parse GOTO statement */
  std::shared_ptr<Statement> parseGoto(const std::vector<Token> &tokens,
                                       size_t &pos);
  
  /** @brief Parse GOSUB statement */
  std::shared_ptr<Statement> parseGosub(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse FOR statement */
  std::shared_ptr<Statement> parseFor(const std::vector<Token> &tokens,
                                      size_t &pos);
  
  /** @brief Parse NEXT statement */
  std::shared_ptr<Statement> parseNext(const std::vector<Token> &tokens,
                                       size_t &pos);
  
  /** @brief Parse DIM statement */
  std::shared_ptr<Statement> parseDim(const std::vector<Token> &tokens,
                                      size_t &pos);
  
  /** @brief Parse DATA statement */
  std::shared_ptr<Statement> parseData(const std::vector<Token> &tokens,
                                       size_t &pos);
  
  /** @brief Parse READ statement */
  std::shared_ptr<Statement> parseRead(const std::vector<Token> &tokens,
                                       size_t &pos);
  
  /** @brief Parse DEF FN statement */
  std::shared_ptr<Statement> parseDef(const std::vector<Token> &tokens,
                                      size_t &pos);
  
  /** @brief Parse ONERR GOTO statement */
  std::shared_ptr<Statement> parseOnErr(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse ON...GOTO/GOSUB statement */
  std::shared_ptr<Statement> parseOn(const std::vector<Token> &tokens,
                                     size_t &pos);
  
  /** @brief Parse RANDOMIZE statement */
  std::shared_ptr<Statement> parseRandomize(const std::vector<Token> &tokens,
                                            size_t &pos);
  
  /** @brief Parse SPEED statement */
  std::shared_ptr<Statement> parseSpeed(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse PR# or IN# device redirection statement */
  std::shared_ptr<Statement>
  parseDeviceRedirect(const std::vector<Token> &tokens, size_t &pos,
                      bool isOutput);
  
  /** @brief Parse WHILE statement */
  std::shared_ptr<Statement> parseWhile(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse WAIT statement */
  std::shared_ptr<Statement> parseWait(const std::vector<Token> &tokens,
                                       size_t &pos);
  
  /** @brief Parse HIMEM statement */
  std::shared_ptr<Statement> parseHimem(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse LOMEM statement */
  std::shared_ptr<Statement> parseLomem(const std::vector<Token> &tokens,
                                        size_t &pos);
  
  /** @brief Parse SHLOAD statement */
  std::shared_ptr<Statement> parseShload(const std::vector<Token> &tokens,
                                         size_t &pos);

  // Helper methods
  
  /**
   * @brief Check if current token matches expected type
   * @param tokens Token sequence
   * @param pos Current position
   * @param type Expected token type
   * @return true if token matches, false otherwise
   */
  bool match(const std::vector<Token> &tokens, size_t pos,
             TokenType type) const;
  
  /**
   * @brief Check if we're at the end of the token sequence
   * @param tokens Token sequence
   * @param pos Current position
   * @return true if at end, false otherwise
   */
  bool isAtEnd(const std::vector<Token> &tokens, size_t pos) const;
};
