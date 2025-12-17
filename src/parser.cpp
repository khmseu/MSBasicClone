#include "parser.h"
#include "float40.h"
#include "functions.h"
#include "interpreter.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <memory>

namespace {
std::string toUpper(const std::string &s) {
  std::string out = s;
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  return out;
}
} // namespace

// Expression classes
class LiteralExpr : public Expression {
public:
  explicit LiteralExpr(const Value &val) : value_(val) {}
  Value evaluate(Interpreter *interp) override { return value_; }

private:
  Value value_;
};

class VariableExpr : public Expression {
public:
  explicit VariableExpr(const std::string &name) : name_(name) {}
  Value evaluate(Interpreter *interp) override {
    return interp->getVariables().getVariable(name_);
  }

private:
  std::string name_;
};

class ArrayAccessExpr : public Expression {
public:
  ArrayAccessExpr(const std::string &name,
                  std::vector<std::shared_ptr<Expression>> indices)
      : name_(name), indices_(std::move(indices)) {}

  Value evaluate(Interpreter *interp) override {
    std::vector<int> idx;
    idx.reserve(indices_.size());
    for (auto &expr : indices_) {
      idx.push_back(static_cast<int>(expr->evaluate(interp).getNumber()));
    }
    return interp->getVariables().getArrayElement(name_, idx);
  }

private:
  std::string name_;
  std::vector<std::shared_ptr<Expression>> indices_;
};

class UnaryExpr : public Expression {
public:
  UnaryExpr(TokenType op, std::shared_ptr<Expression> operand)
      : op_(op), operand_(std::move(operand)) {}

  Value evaluate(Interpreter *interp) override {
    Value v = operand_->evaluate(interp);
    if (op_ == TokenType::MINUS) {
      return Value(-v.getNumber());
    }
    // Unary plus is no-op
    return v;
  }

private:
  TokenType op_;
  std::shared_ptr<Expression> operand_;
};

class BinaryExpr : public Expression {
public:
  BinaryExpr(std::shared_ptr<Expression> left, TokenType op,
             std::shared_ptr<Expression> right)
      : left_(left), op_(op), right_(right) {}

  Value evaluate(Interpreter *interp) override {
    Value lval = left_->evaluate(interp);
    Value rval = right_->evaluate(interp);

    switch (op_) {
    case TokenType::PLUS:
      return lval + rval;
    case TokenType::MINUS:
      return lval - rval;
    case TokenType::MULTIPLY:
      return lval * rval;
    case TokenType::DIVIDE:
      return lval / rval;
    case TokenType::POWER: {
      Float40 a(lval.getNumber());
      Float40 b(rval.getNumber());
      return Value(a.power(b).toDouble());
    }
    case TokenType::MOD: {
      Float40 a(lval.getNumber());
      Float40 b(rval.getNumber());
      return Value(a.mod(b).toDouble());
    }
    case TokenType::EQUAL:
      return Value(lval == rval ? 1.0 : 0.0);
    case TokenType::NOT_EQUAL:
      return Value(lval != rval ? 1.0 : 0.0);
    case TokenType::LESS:
      return Value(lval < rval ? 1.0 : 0.0);
    case TokenType::GREATER:
      return Value(lval > rval ? 1.0 : 0.0);
    case TokenType::LESS_EQUAL:
      return Value(lval <= rval ? 1.0 : 0.0);
    case TokenType::GREATER_EQUAL:
      return Value(lval >= rval ? 1.0 : 0.0);
    case TokenType::AND:
      return Value((lval.getNumber() != 0 && rval.getNumber() != 0) ? 1.0
                                                                    : 0.0);
    case TokenType::OR:
      return Value((lval.getNumber() != 0 || rval.getNumber() != 0) ? 1.0
                                                                    : 0.0);
    default:
      return Value(0.0);
    }
  }

private:
  std::shared_ptr<Expression> left_;
  TokenType op_;
  std::shared_ptr<Expression> right_;
};

class UserFunctionCallExpr : public Expression {
public:
  UserFunctionCallExpr(std::string name, std::shared_ptr<Expression> arg)
      : name_(std::move(name)), arg_(std::move(arg)) {}

  Value evaluate(Interpreter *interp) override {
    auto &vars = interp->getVariables();
    const auto &fn = vars.getFunction(name_);

    Value argValue = arg_->evaluate(interp);

    bool hadExisting = vars.hasVariable(fn.parameter);
    Value oldValue = vars.getVariable(fn.parameter);

    vars.setVariable(fn.parameter, argValue);
    Value result = fn.body->evaluate(interp);

    if (hadExisting) {
      vars.setVariable(fn.parameter, oldValue);
    } else {
      vars.unsetVariable(fn.parameter);
    }

    return result;
  }

private:
  std::string name_;
  std::shared_ptr<Expression> arg_;
};

class FunctionCallExpr : public Expression {
public:
  FunctionCallExpr(TokenType func,
                   std::vector<std::shared_ptr<Expression>> args)
      : func_(func), args_(args) {}

  Value evaluate(Interpreter *interp) override {
    std::vector<Value> argValues;
    for (auto &arg : args_) {
      argValues.push_back(arg->evaluate(interp));
    }

    switch (func_) {
    case TokenType::SIN:
      return funcSin(argValues[0]);
    case TokenType::COS:
      return funcCos(argValues[0]);
    case TokenType::TAN:
      return funcTan(argValues[0]);
    case TokenType::ATN:
      return funcAtn(argValues[0]);
    case TokenType::EXP:
      return funcExp(argValues[0]);
    case TokenType::LOG:
      return funcLog(argValues[0]);
    case TokenType::SQR:
      return funcSqr(argValues[0]);
    case TokenType::ABS:
      return funcAbs(argValues[0]);
    case TokenType::INT:
      return funcInt(argValues[0]);
    case TokenType::SGN:
      return funcSgn(argValues[0]);
    case TokenType::RND:
      return funcRnd(argValues[0]);
    case TokenType::LEN:
      return funcLen(argValues[0]);
    case TokenType::VAL:
      return funcVal(argValues[0]);
    case TokenType::ASC:
      return funcAsc(argValues[0]);
    case TokenType::CHR:
      return funcChr(argValues[0]);
    case TokenType::LEFT:
      return funcLeft(argValues[0], argValues[1]);
    case TokenType::RIGHT:
      return funcRight(argValues[0], argValues[1]);
    case TokenType::MID:
      return funcMid(argValues[0], argValues[1], argValues[2]);
    case TokenType::STR:
      return funcStr(argValues[0]);
    default:
      return Value(0.0);
    }
  }

private:
  TokenType func_;
  std::vector<std::shared_ptr<Expression>> args_;
};

// Statement classes
class PrintStmt : public Statement {
public:
  PrintStmt(std::vector<std::shared_ptr<Expression>> exprs,
            std::vector<bool> newlines)
      : exprs_(exprs), newlines_(newlines) {}

  void execute(Interpreter *interp) override {
    for (size_t i = 0; i < exprs_.size(); ++i) {
      Value val = exprs_[i]->evaluate(interp);
      std::cout << val.getString();
      if (i < newlines_.size() && newlines_[i]) {
        std::cout << "\n";
      }
    }
    // Print final newline unless last separator was ; or ,
    if (!newlines_.empty() && newlines_.back()) {
      // Already printed above
    } else if (!exprs_.empty() && (newlines_.empty() || !newlines_.back())) {
      // Ended with separator, no final newline
    } else {
      std::cout << "\n";
    }
  }

private:
  std::vector<std::shared_ptr<Expression>> exprs_;
  std::vector<bool> newlines_;
};

class LetStmt : public Statement {
public:
  LetStmt(const std::string &var, std::shared_ptr<Expression> expr)
      : var_(var), expr_(expr) {}

  void execute(Interpreter *interp) override {
    Value val = expr_->evaluate(interp);
    interp->getVariables().setVariable(var_, val);
  }

private:
  std::string var_;
  std::shared_ptr<Expression> expr_;
};

class ArrayLetStmt : public Statement {
public:
  ArrayLetStmt(const std::string &var,
               std::vector<std::shared_ptr<Expression>> indices,
               std::shared_ptr<Expression> expr)
      : var_(var), indices_(std::move(indices)), expr_(std::move(expr)) {}

  void execute(Interpreter *interp) override {
    std::vector<int> idx;
    idx.reserve(indices_.size());
    for (auto &e : indices_) {
      idx.push_back(static_cast<int>(e->evaluate(interp).getNumber()));
    }
    Value val = expr_->evaluate(interp);
    interp->getVariables().setArrayElement(var_, idx, val);
  }

private:
  std::string var_;
  std::vector<std::shared_ptr<Expression>> indices_;
  std::shared_ptr<Expression> expr_;
};

class EndStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->endProgram(); }
};

class GotoStmt : public Statement {
public:
  explicit GotoStmt(int lineNum) : lineNum_(lineNum) {}
  void execute(Interpreter *interp) override { interp->gotoLine(lineNum_); }

private:
  int lineNum_;
};

class GosubStmt : public Statement {
public:
  explicit GosubStmt(int lineNum) : lineNum_(lineNum) {}
  void execute(Interpreter *interp) override { interp->gosub(lineNum_); }

private:
  int lineNum_;
};

class ReturnStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->returnFromGosub(); }
};

class IfStmt : public Statement {
public:
  IfStmt(std::shared_ptr<Expression> condition,
         std::vector<std::shared_ptr<Statement>> thenStmts,
         std::vector<std::shared_ptr<Statement>> elseStmts = {})
      : condition_(condition), thenStmts_(thenStmts), elseStmts_(elseStmts) {}

  void execute(Interpreter *interp) override {
    Value val = condition_->evaluate(interp);
    if (val.getNumber() != 0) {
      for (auto &stmt : thenStmts_) {
        stmt->execute(interp);
      }
    } else if (!elseStmts_.empty()) {
      for (auto &stmt : elseStmts_) {
        stmt->execute(interp);
      }
    }
  }

private:
  std::shared_ptr<Expression> condition_;
  std::vector<std::shared_ptr<Statement>> thenStmts_;
  std::vector<std::shared_ptr<Statement>> elseStmts_;
};

class ForStmt : public Statement {
public:
  ForStmt(const std::string &var, std::shared_ptr<Expression> start,
          std::shared_ptr<Expression> end, std::shared_ptr<Expression> step)
      : var_(var), start_(start), end_(end), step_(step) {}

  void execute(Interpreter *interp) override {
    double startVal = start_->evaluate(interp).getNumber();
    double endVal = end_->evaluate(interp).getNumber();
    double stepVal = step_ ? step_->evaluate(interp).getNumber() : 1.0;

    interp->getVariables().setVariable(var_, Value(startVal));
    interp->pushForLoop(var_, endVal, stepVal, interp->getCurrentLine());
  }

private:
  std::string var_;
  std::shared_ptr<Expression> start_;
  std::shared_ptr<Expression> end_;
  std::shared_ptr<Expression> step_;
};

class NextStmt : public Statement {
public:
  explicit NextStmt(const std::string &var) : var_(var) {}
  void execute(Interpreter *interp) override { interp->nextForLoop(var_); }

private:
  std::string var_;
};

class InputStmt : public Statement {
public:
  InputStmt(const std::string &prompt, const std::vector<std::string> &vars)
      : prompt_(prompt), vars_(vars) {}

  void execute(Interpreter *interp) override {
    if (!prompt_.empty()) {
      std::cout << prompt_;
    }

    for (const auto &var : vars_) {
      std::string input;
      std::cout << "? ";
      std::getline(std::cin, input);

      // Try to parse as number, otherwise treat as string
      if (!var.empty() && var.back() == '$') {
        // String variable
        interp->getVariables().setVariable(var, Value(input));
      } else {
        // Numeric variable
        try {
          double val = std::stod(input);
          interp->getVariables().setVariable(var, Value(val));
        } catch (...) {
          std::cout << "?REENTER\n";
        }
      }
    }
  }

private:
  std::string prompt_;
  std::vector<std::string> vars_;
};

class DimStmt : public Statement {
public:
  struct Entry {
    std::string name;
    std::vector<std::shared_ptr<Expression>> dimensions;
  };

  explicit DimStmt(std::vector<Entry> entries) : entries_(std::move(entries)) {}

  void execute(Interpreter *interp) override {
    for (auto &entry : entries_) {
      std::vector<int> dims;
      dims.reserve(entry.dimensions.size());
      for (auto &expr : entry.dimensions) {
        dims.push_back(static_cast<int>(expr->evaluate(interp).getNumber()));
      }
      interp->getVariables().dimArray(entry.name, dims);
    }
  }

private:
  std::vector<Entry> entries_;
};

class DataStmt : public Statement {
public:
  explicit DataStmt(std::vector<Value> values) : values_(std::move(values)) {}

  void execute(Interpreter *interp) override {
    if (interp->isImmediateMode()) {
      for (const auto &v : values_) {
        interp->addDataValue(v);
      }
    }
  }

  void collectData(std::vector<Value> &out) const override {
    out.insert(out.end(), values_.begin(), values_.end());
  }

private:
  std::vector<Value> values_;
};

class ReadStmt : public Statement {
public:
  struct Target {
    std::string name;
    std::vector<std::shared_ptr<Expression>> indices;
  };

  explicit ReadStmt(std::vector<Target> targets)
      : targets_(std::move(targets)) {}

  void execute(Interpreter *interp) override {
    for (auto &target : targets_) {
      Value v = interp->readData();
      if (target.indices.empty()) {
        interp->getVariables().setVariable(target.name, v);
      } else {
        std::vector<int> idx;
        idx.reserve(target.indices.size());
        for (auto &expr : target.indices) {
          idx.push_back(static_cast<int>(expr->evaluate(interp).getNumber()));
        }
        interp->getVariables().setArrayElement(target.name, idx, v);
      }
    }
  }

private:
  std::vector<Target> targets_;
};

class RestoreStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->restoreData(); }
};

class OnErrStmt : public Statement {
public:
  explicit OnErrStmt(int lineNum) : lineNum_(lineNum) {}
  void execute(Interpreter *interp) override {
    interp->setErrorHandler(lineNum_);
  }

private:
  int lineNum_;
};

class ResumeStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->resume(); }
};

class DefStmt : public Statement {
public:
  DefStmt(std::string name, std::string param, std::shared_ptr<Expression> expr)
      : name_(std::move(name)), param_(std::move(param)),
        expr_(std::move(expr)) {}

  void execute(Interpreter *interp) override {
    interp->getVariables().defineFunction(name_, param_, expr_);
  }

private:
  std::string name_;
  std::string param_;
  std::shared_ptr<Expression> expr_;
};

class RemStmt : public Statement {
public:
  void execute(Interpreter *) override {
    // REM does nothing - it's a comment
  }
};

// Parser implementation
Parser::Parser() {}

std::vector<std::shared_ptr<Statement>>
Parser::parse(const std::vector<Token> &tokens) {
  std::vector<std::shared_ptr<Statement>> statements;
  size_t pos = 0;

  while (pos < tokens.size()) {
    // Skip newlines
    if (tokens[pos].type == TokenType::NEWLINE) {
      pos++;
      continue;
    }

    // Check for statement separator
    if (tokens[pos].type == TokenType::COLON) {
      pos++;
      continue;
    }

    auto stmt = parseStatement(tokens, pos);
    if (stmt) {
      statements.push_back(stmt);
    }
  }

  return statements;
}

std::shared_ptr<Statement>
Parser::parseStatement(const std::vector<Token> &tokens, size_t &pos) {
  if (pos >= tokens.size())
    return nullptr;

  Token &token = const_cast<Token &>(tokens[pos]);

  switch (token.type) {
  case TokenType::PRINT:
    return parsePrint(tokens, pos);
  case TokenType::LET:
    return parseLetOrAssignment(tokens, pos);
  case TokenType::END:
    pos++;
    return std::make_shared<EndStmt>();
  case TokenType::IF:
    return parseIf(tokens, pos);
  case TokenType::GOTO:
    return parseGoto(tokens, pos);
  case TokenType::GOSUB:
    return parseGosub(tokens, pos);
  case TokenType::RETURN:
    pos++;
    return std::make_shared<ReturnStmt>();
  case TokenType::FOR:
    return parseFor(tokens, pos);
  case TokenType::NEXT:
    return parseNext(tokens, pos);
  case TokenType::INPUT:
    return parseInput(tokens, pos);
  case TokenType::DIM:
    return parseDim(tokens, pos);
  case TokenType::DATA:
    return parseData(tokens, pos);
  case TokenType::READ:
    return parseRead(tokens, pos);
  case TokenType::RESTORE:
    pos++;
    return std::make_shared<RestoreStmt>();
  case TokenType::DEF:
    return parseDef(tokens, pos);
  case TokenType::REM:
    // REM consumes rest of line
    pos++;
    while (pos < tokens.size() && tokens[pos].type != TokenType::NEWLINE &&
           tokens[pos].type != TokenType::COLON) {
      pos++;
    }
    return std::make_shared<RemStmt>();
  case TokenType::ONERR:
    return parseOnErr(tokens, pos);
  case TokenType::RESUME:
    pos++;
    return std::make_shared<ResumeStmt>();
  case TokenType::IDENTIFIER:
    return parseLetOrAssignment(tokens, pos);
  default:
    pos++;
    return nullptr;
  }
}

std::shared_ptr<Statement> Parser::parsePrint(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip PRINT

  std::vector<std::shared_ptr<Expression>> exprs;
  std::vector<bool> newlines;
  bool endsWithSeparator = false;

  while (pos < tokens.size() && tokens[pos].type != TokenType::COLON &&
         tokens[pos].type != TokenType::NEWLINE) {
    auto expr = parseExpression(tokens, pos);
    exprs.push_back(expr);

    endsWithSeparator = false;
    if (pos < tokens.size()) {
      if (tokens[pos].type == TokenType::SEMICOLON) {
        newlines.push_back(false);
        endsWithSeparator = true;
        pos++;
      } else if (tokens[pos].type == TokenType::COMMA) {
        newlines.push_back(false);
        endsWithSeparator = true;
        pos++;
      } else {
        newlines.push_back(true);
        break;
      }
    } else {
      newlines.push_back(true);
    }
  }

  // If the statement ended with a separator, don't add final newline
  if (endsWithSeparator && !newlines.empty()) {
    newlines.back() = false;
  }

  return std::make_shared<PrintStmt>(exprs, newlines);
}

std::shared_ptr<Statement>
Parser::parseLetOrAssignment(const std::vector<Token> &tokens, size_t &pos) {
  if (tokens[pos].type == TokenType::LET) {
    pos++; // Skip LET
  }

  if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
    throw std::runtime_error("SYNTAX ERROR");
  }

  std::string varName = tokens[pos].text;
  pos++;

  std::vector<std::shared_ptr<Expression>> indices;
  if (pos < tokens.size() && tokens[pos].type == TokenType::LPAREN) {
    pos++;
    while (pos < tokens.size() && tokens[pos].type != TokenType::RPAREN) {
      indices.push_back(parseExpression(tokens, pos));
      if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
        pos++;
      } else {
        break;
      }
    }
    if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED )");
    }
    pos++;
  }

  if (pos >= tokens.size() || tokens[pos].type != TokenType::EQUAL) {
    throw std::runtime_error("SYNTAX ERROR");
  }
  pos++;

  auto expr = parseExpression(tokens, pos);
  if (indices.empty()) {
    return std::make_shared<LetStmt>(varName, expr);
  }
  return std::make_shared<ArrayLetStmt>(varName, indices, expr);
}

std::shared_ptr<Expression>
Parser::parseExpression(const std::vector<Token> &tokens, size_t &pos) {
  return parseOrExpression(tokens, pos);
}

std::shared_ptr<Expression>
Parser::parseOrExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parseAndExpression(tokens, pos);

  while (pos < tokens.size() && tokens[pos].type == TokenType::OR) {
    pos++;
    auto right = parseAndExpression(tokens, pos);
    left = std::make_shared<BinaryExpr>(left, TokenType::OR, right);
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parseAndExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parseRelationalExpression(tokens, pos);

  while (pos < tokens.size() && tokens[pos].type == TokenType::AND) {
    pos++;
    auto right = parseRelationalExpression(tokens, pos);
    left = std::make_shared<BinaryExpr>(left, TokenType::AND, right);
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parseNotExpression(const std::vector<Token> &tokens, size_t &pos) {
  return parseRelationalExpression(tokens, pos);
}

std::shared_ptr<Expression>
Parser::parseRelationalExpression(const std::vector<Token> &tokens,
                                  size_t &pos) {
  auto left = parseAdditiveExpression(tokens, pos);

  if (pos < tokens.size()) {
    TokenType op = tokens[pos].type;
    if (op == TokenType::EQUAL || op == TokenType::NOT_EQUAL ||
        op == TokenType::LESS || op == TokenType::GREATER ||
        op == TokenType::LESS_EQUAL || op == TokenType::GREATER_EQUAL) {
      pos++;
      auto right = parseAdditiveExpression(tokens, pos);
      return std::make_shared<BinaryExpr>(left, op, right);
    }
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parseAdditiveExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parseMultiplicativeExpression(tokens, pos);

  while (pos < tokens.size()) {
    TokenType op = tokens[pos].type;
    if (op == TokenType::PLUS || op == TokenType::MINUS) {
      pos++;
      auto right = parseMultiplicativeExpression(tokens, pos);
      left = std::make_shared<BinaryExpr>(left, op, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parseMultiplicativeExpression(const std::vector<Token> &tokens,
                                      size_t &pos) {
  auto left = parseUnaryExpression(tokens, pos);

  while (pos < tokens.size()) {
    TokenType op = tokens[pos].type;
    if (op == TokenType::MULTIPLY || op == TokenType::DIVIDE ||
        op == TokenType::MOD) {
      pos++;
      auto right = parseUnaryExpression(tokens, pos);
      left = std::make_shared<BinaryExpr>(left, op, right);
    } else {
      break;
    }
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parseUnaryExpression(const std::vector<Token> &tokens, size_t &pos) {
  if (pos < tokens.size() && (tokens[pos].type == TokenType::PLUS ||
                              tokens[pos].type == TokenType::MINUS)) {
    TokenType op = tokens[pos].type;
    pos++;
    auto operand = parseUnaryExpression(tokens, pos);
    return std::make_shared<UnaryExpr>(op, operand);
  }

  return parsePowerExpression(tokens, pos);
}

std::shared_ptr<Expression>
Parser::parsePowerExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parsePrimaryExpression(tokens, pos);

  if (pos < tokens.size() && tokens[pos].type == TokenType::POWER) {
    pos++;
    auto right = parsePowerExpression(tokens, pos);
    left = std::make_shared<BinaryExpr>(left, TokenType::POWER, right);
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parsePrimaryExpression(const std::vector<Token> &tokens, size_t &pos) {
  if (pos >= tokens.size()) {
    throw std::runtime_error("SYNTAX ERROR");
  }

  const Token &token = tokens[pos];

  if (token.type == TokenType::NUMBER) {
    pos++;
    return std::make_shared<LiteralExpr>(token.value);
  }

  if (token.type == TokenType::STRING) {
    pos++;
    return std::make_shared<LiteralExpr>(token.value);
  }

  if (token.type == TokenType::IDENTIFIER) {
    std::string name = token.text;
    std::string upperName = toUpper(name);
    pos++;

    bool hasParen =
        pos < tokens.size() && tokens[pos].type == TokenType::LPAREN;
    bool isUserFn = upperName.rfind("FN", 0) == 0;

    if (hasParen) {
      pos++; // Skip '('
      std::vector<std::shared_ptr<Expression>> indices;
      while (pos < tokens.size() && tokens[pos].type != TokenType::RPAREN) {
        indices.push_back(parseExpression(tokens, pos));
        if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
          pos++;
        } else {
          break;
        }
      }
      if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
        throw std::runtime_error("SYNTAX ERROR: MISSING )");
      }
      pos++;

      if (isUserFn) {
        if (indices.size() != 1) {
          throw std::runtime_error("SYNTAX ERROR: FN EXPECTS 1 ARGUMENT");
        }
        return std::make_shared<UserFunctionCallExpr>(upperName,
                                                      indices.front());
      }

      return std::make_shared<ArrayAccessExpr>(name, indices);
    }

    if (isUserFn) {
      throw std::runtime_error("SYNTAX ERROR");
    }

    return std::make_shared<VariableExpr>(name);
  }

  if (token.type == TokenType::LPAREN) {
    pos++;
    auto expr = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
      throw std::runtime_error("SYNTAX ERROR: MISSING )");
    }
    pos++;
    return expr;
  }

  // Built-in functions - single argument
  if (token.type == TokenType::SIN || token.type == TokenType::COS ||
      token.type == TokenType::TAN || token.type == TokenType::ATN ||
      token.type == TokenType::EXP || token.type == TokenType::LOG ||
      token.type == TokenType::SQR || token.type == TokenType::ABS ||
      token.type == TokenType::INT || token.type == TokenType::SGN ||
      token.type == TokenType::RND || token.type == TokenType::LEN ||
      token.type == TokenType::VAL || token.type == TokenType::ASC ||
      token.type == TokenType::CHR || token.type == TokenType::STR) {
    TokenType func = token.type;
    pos++;

    if (pos >= tokens.size() || tokens[pos].type != TokenType::LPAREN) {
      throw std::runtime_error("SYNTAX ERROR");
    }
    pos++;

    std::vector<std::shared_ptr<Expression>> args;
    args.push_back(parseExpression(tokens, pos));

    if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
      throw std::runtime_error("SYNTAX ERROR: MISSING )");
    }
    pos++;

    return std::make_shared<FunctionCallExpr>(func, args);
  }

  // Built-in functions - two arguments
  if (token.type == TokenType::LEFT || token.type == TokenType::RIGHT) {
    TokenType func = token.type;
    pos++;

    if (pos >= tokens.size() || tokens[pos].type != TokenType::LPAREN) {
      throw std::runtime_error("SYNTAX ERROR");
    }
    pos++;

    std::vector<std::shared_ptr<Expression>> args;
    args.push_back(parseExpression(tokens, pos));

    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;

    args.push_back(parseExpression(tokens, pos));

    if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
      throw std::runtime_error("SYNTAX ERROR: MISSING )");
    }
    pos++;

    return std::make_shared<FunctionCallExpr>(func, args);
  }

  // MID$ - three arguments
  if (token.type == TokenType::MID) {
    TokenType func = token.type;
    pos++;

    if (pos >= tokens.size() || tokens[pos].type != TokenType::LPAREN) {
      throw std::runtime_error("SYNTAX ERROR");
    }
    pos++;

    std::vector<std::shared_ptr<Expression>> args;
    args.push_back(parseExpression(tokens, pos));

    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;

    args.push_back(parseExpression(tokens, pos));

    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;

    args.push_back(parseExpression(tokens, pos));

    if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
      throw std::runtime_error("SYNTAX ERROR: MISSING )");
    }
    pos++;

    return std::make_shared<FunctionCallExpr>(func, args);
  }

  throw std::runtime_error("SYNTAX ERROR");
}

std::shared_ptr<Statement> Parser::parseInput(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip INPUT

  std::string prompt;
  std::vector<std::string> vars;

  // Check for prompt string
  if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
    prompt = tokens[pos].text;
    pos++;

    if (pos < tokens.size() && tokens[pos].type == TokenType::SEMICOLON) {
      pos++;
    }
  }

  // Parse variable list
  while (pos < tokens.size() && tokens[pos].type != TokenType::COLON &&
         tokens[pos].type != TokenType::NEWLINE) {
    if (tokens[pos].type == TokenType::IDENTIFIER) {
      vars.push_back(tokens[pos].text);
      pos++;

      if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
        pos++;
      }
    } else {
      break;
    }
  }

  return std::make_shared<InputStmt>(prompt, vars);
}

std::shared_ptr<Statement> Parser::parseIf(const std::vector<Token> &tokens,
                                           size_t &pos) {
  pos++; // Skip IF

  auto condition = parseExpression(tokens, pos);

  if (pos >= tokens.size() || tokens[pos].type != TokenType::THEN) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED THEN");
  }
  pos++;

  std::vector<std::shared_ptr<Statement>> thenStmts;
  std::vector<std::shared_ptr<Statement>> elseStmts;

  // Check if THEN is followed by a line number (GOTO)
  if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
    int lineNum = static_cast<int>(tokens[pos].value.getNumber());
    pos++;
    thenStmts.push_back(std::make_shared<GotoStmt>(lineNum));
  } else {
    // Parse statements until ELSE or end of line
    while (pos < tokens.size() && tokens[pos].type != TokenType::ELSE &&
           tokens[pos].type != TokenType::COLON &&
           tokens[pos].type != TokenType::NEWLINE) {
      auto stmt = parseStatement(tokens, pos);
      if (stmt) {
        thenStmts.push_back(stmt);
      }
    }
  }

  // Check for ELSE
  if (pos < tokens.size() && tokens[pos].type == TokenType::ELSE) {
    pos++;

    if (pos < tokens.size() && tokens[pos].type == TokenType::NUMBER) {
      int lineNum = static_cast<int>(tokens[pos].value.getNumber());
      pos++;
      elseStmts.push_back(std::make_shared<GotoStmt>(lineNum));
    } else {
      while (pos < tokens.size() && tokens[pos].type != TokenType::COLON &&
             tokens[pos].type != TokenType::NEWLINE) {
        auto stmt = parseStatement(tokens, pos);
        if (stmt) {
          elseStmts.push_back(stmt);
        }
      }
    }
  }

  return std::make_shared<IfStmt>(condition, thenStmts, elseStmts);
}

std::shared_ptr<Statement> Parser::parseGoto(const std::vector<Token> &tokens,
                                             size_t &pos) {
  pos++; // Skip GOTO

  if (pos >= tokens.size() || tokens[pos].type != TokenType::NUMBER) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED LINE NUMBER");
  }

  int lineNum = static_cast<int>(tokens[pos].value.getNumber());
  pos++;

  return std::make_shared<GotoStmt>(lineNum);
}

std::shared_ptr<Statement> Parser::parseGosub(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip GOSUB

  if (pos >= tokens.size() || tokens[pos].type != TokenType::NUMBER) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED LINE NUMBER");
  }

  int lineNum = static_cast<int>(tokens[pos].value.getNumber());
  pos++;

  return std::make_shared<GosubStmt>(lineNum);
}

std::shared_ptr<Statement> Parser::parseFor(const std::vector<Token> &tokens,
                                            size_t &pos) {
  pos++; // Skip FOR

  if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED VARIABLE");
  }

  std::string varName = tokens[pos].text;
  pos++;

  if (pos >= tokens.size() || tokens[pos].type != TokenType::EQUAL) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED =");
  }
  pos++;

  auto start = parseExpression(tokens, pos);

  if (pos >= tokens.size() || tokens[pos].type != TokenType::TO) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED TO");
  }
  pos++;

  auto end = parseExpression(tokens, pos);

  std::shared_ptr<Expression> step;
  if (pos < tokens.size() && tokens[pos].type == TokenType::STEP) {
    pos++;
    step = parseExpression(tokens, pos);
  }

  return std::make_shared<ForStmt>(varName, start, end, step);
}

std::shared_ptr<Statement> Parser::parseNext(const std::vector<Token> &tokens,
                                             size_t &pos) {
  pos++; // Skip NEXT

  std::string varName;
  if (pos < tokens.size() && tokens[pos].type == TokenType::IDENTIFIER) {
    varName = tokens[pos].text;
    pos++;
  }

  return std::make_shared<NextStmt>(varName);
}

std::shared_ptr<Statement> Parser::parseDim(const std::vector<Token> &tokens,
                                            size_t &pos) {
  pos++; // Skip DIM

  std::vector<DimStmt::Entry> entries;

  while (pos < tokens.size() && tokens[pos].type != TokenType::NEWLINE &&
         tokens[pos].type != TokenType::COLON) {
    if (tokens[pos].type != TokenType::IDENTIFIER) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED ARRAY NAME");
    }

    DimStmt::Entry entry;
    entry.name = tokens[pos].text;
    pos++;

    if (pos >= tokens.size() || tokens[pos].type != TokenType::LPAREN) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED (");
    }
    pos++; // Skip '('

    while (pos < tokens.size() && tokens[pos].type != TokenType::RPAREN) {
      entry.dimensions.push_back(parseExpression(tokens, pos));
      if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
        pos++;
      } else {
        break;
      }
    }

    if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED )");
    }
    pos++; // Skip ')'

    entries.push_back(std::move(entry));

    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++;
      continue;
    }
    break;
  }

  return std::make_shared<DimStmt>(entries);
}

std::shared_ptr<Statement> Parser::parseData(const std::vector<Token> &tokens,
                                             size_t &pos) {
  pos++; // Skip DATA
  std::vector<Value> values;

  while (pos < tokens.size() && tokens[pos].type != TokenType::NEWLINE &&
         tokens[pos].type != TokenType::COLON) {
    if (tokens[pos].type == TokenType::NUMBER ||
        tokens[pos].type == TokenType::STRING) {
      values.push_back(tokens[pos].value);
      pos++;
      if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
        pos++;
      }
      continue;
    }
    throw std::runtime_error("SYNTAX ERROR IN DATA");
  }

  return std::make_shared<DataStmt>(values);
}

std::shared_ptr<Statement> Parser::parseRead(const std::vector<Token> &tokens,
                                             size_t &pos) {
  pos++; // Skip READ

  std::vector<ReadStmt::Target> targets;

  while (pos < tokens.size() && tokens[pos].type != TokenType::NEWLINE &&
         tokens[pos].type != TokenType::COLON) {
    if (tokens[pos].type != TokenType::IDENTIFIER) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED VARIABLE");
    }

    ReadStmt::Target tgt;
    tgt.name = tokens[pos].text;
    pos++;

    if (pos < tokens.size() && tokens[pos].type == TokenType::LPAREN) {
      pos++; // Skip '('
      while (pos < tokens.size() && tokens[pos].type != TokenType::RPAREN) {
        tgt.indices.push_back(parseExpression(tokens, pos));
        if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
          pos++;
        } else {
          break;
        }
      }
      if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
        throw std::runtime_error("SYNTAX ERROR: EXPECTED )");
      }
      pos++;
    }

    targets.push_back(std::move(tgt));

    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++;
      continue;
    }
    break;
  }

  return std::make_shared<ReadStmt>(targets);
}

std::shared_ptr<Statement> Parser::parseDef(const std::vector<Token> &tokens,
                                            size_t &pos) {
  pos++; // Skip DEF

  if (pos >= tokens.size()) {
    throw std::runtime_error("SYNTAX ERROR");
  }

  std::string fnName;
  if (tokens[pos].type == TokenType::FN) {
    pos++;
    if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FUNCTION NAME");
    }
    fnName = "FN" + tokens[pos].text;
    pos++;
  } else if (tokens[pos].type == TokenType::IDENTIFIER) {
    fnName = tokens[pos].text;
    pos++;
  } else {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED FUNCTION NAME");
  }

  if (toUpper(fnName).rfind("FN", 0) != 0) {
    throw std::runtime_error("SYNTAX ERROR: FUNCTION MUST START WITH FN");
  }

  if (pos >= tokens.size() || tokens[pos].type != TokenType::LPAREN) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED (");
  }
  pos++;

  if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED PARAMETER");
  }

  std::string param = tokens[pos].text;
  pos++;

  if (pos >= tokens.size() || tokens[pos].type != TokenType::RPAREN) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED )");
  }
  pos++;

  if (pos >= tokens.size() || tokens[pos].type != TokenType::EQUAL) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED =");
  }
  pos++;

  auto expr = parseExpression(tokens, pos);
  return std::make_shared<DefStmt>(toUpper(fnName), param, expr);
}

std::shared_ptr<Statement>
Parser::parseOnErr(const std::vector<Token> &tokens, size_t &pos) {
  pos++; // Skip ONERR

  if (pos >= tokens.size() || tokens[pos].type != TokenType::GOTO) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED GOTO");
  }
  pos++;

  if (pos >= tokens.size() || tokens[pos].type != TokenType::NUMBER) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED LINE NUMBER");
  }

  int lineNum = static_cast<int>(tokens[pos].value.getNumber());
  pos++;

  return std::make_shared<OnErrStmt>(lineNum);
}

bool Parser::match(const std::vector<Token> &tokens, size_t pos,
                   TokenType type) const {
  return pos < tokens.size() && tokens[pos].type == type;
}

bool Parser::isAtEnd(const std::vector<Token> &tokens, size_t pos) const {
  return pos >= tokens.size();
}
