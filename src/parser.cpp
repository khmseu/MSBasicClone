/**
 * @file parser.cpp
 * @brief Implementation of recursive descent parser for Applesoft BASIC
 * 
 * This file implements the Parser class which converts tokens into an
 * Abstract Syntax Tree (AST) of Expression and Statement nodes.
 * 
 * Parser structure:
 * - parse(): Entry point, parses multiple statements separated by colons
 * - parseStatement(): Dispatches to specific statement parsers
 * - parseExpression(): Expression parser with operator precedence
 * - parsePrimary(): Literals, variables, function calls, parentheses
 * 
 * Operator precedence (lowest to highest):
 * 1. OR (logical or)
 * 2. AND (logical and)
 * 3. NOT (logical not)
 * 4. Relational (=, <>, <, >, <=, >=)
 * 5. Additive (+, -)
 * 6. Multiplicative (*, /, MOD)
 * 7. Unary (+, -)
 * 8. Power (^)
 * 9. Primary (literals, identifiers, function calls)
 * 
 * Statement implementations:
 * - Simple statements implemented as inline classes (StopStmt, PlotStmt, etc.)
 * - Complex statements have dedicated classes in statements.cpp
 * 
 * The parser uses a Pratt parser approach for expression parsing with
 * explicit precedence levels for each operator type.
 */

#include "parser.h"
#include "float40.h"
#include "functions.h"
#include "graphics.h"
#include "interpreter.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <utility>

namespace {
/**
 * @brief Convert string to uppercase
 * 
 * Helper function for case-insensitive keyword matching.
 * 
 * @param s Input string
 * @return Uppercase version of input string
 */
std::string toUpper(const std::string &s) {
  std::string out = s;
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) {
    return static_cast<char>(std::toupper(c));
  });
  return out;
}
} // namespace

// Forward declarations for statements used in parseStatement
class StopStmt;
class PlotStmt;
class HlinStmt;
class VlinStmt;
class OnTransferStmt;
class HtabStmt;
class VtabStmt;
class InverseStmt;
class NormalStmt;
class FlashStmt;
class HgrStmt;
class HcolorStmt;

// Statements used by parser
class StopStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->stop(); }
};

class PlotStmt : public Statement {
public:
  PlotStmt(std::shared_ptr<Expression> x, std::shared_ptr<Expression> y)
      : x_(std::move(x)), y_(std::move(y)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    graphics().plot(x_->evaluate(interp).getNumber(),
                    y_->evaluate(interp).getNumber());
  }

private:
  std::shared_ptr<Expression> x_;
  std::shared_ptr<Expression> y_;
};

class HlinStmt : public Statement {
public:
  HlinStmt(std::shared_ptr<Expression> x1, std::shared_ptr<Expression> x2,
           std::shared_ptr<Expression> y)
      : x1_(std::move(x1)), x2_(std::move(x2)), y_(std::move(y)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    graphics().hlin(x1_->evaluate(interp).getNumber(),
                    x2_->evaluate(interp).getNumber(),
                    y_->evaluate(interp).getNumber());
  }

private:
  std::shared_ptr<Expression> x1_;
  std::shared_ptr<Expression> x2_;
  std::shared_ptr<Expression> y_;
};

class VlinStmt : public Statement {
public:
  VlinStmt(std::shared_ptr<Expression> y1, std::shared_ptr<Expression> y2,
           std::shared_ptr<Expression> x)
      : y1_(std::move(y1)), y2_(std::move(y2)), x_(std::move(x)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    graphics().vlin(y1_->evaluate(interp).getNumber(),
                    y2_->evaluate(interp).getNumber(),
                    x_->evaluate(interp).getNumber());
  }

private:
  std::shared_ptr<Expression> y1_;
  std::shared_ptr<Expression> y2_;
  std::shared_ptr<Expression> x_;
};

class HplotStmt : public Statement {
public:
  HplotStmt(std::vector<
            std::pair<std::shared_ptr<Expression>, std::shared_ptr<Expression>>>
                coords)
      : coords_(std::move(coords)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    // HPLOT plots points in high-resolution graphics mode
    if (!coords_.empty()) {
      double x = coords_[0].first->evaluate(interp).getNumber();
      double y = coords_[0].second->evaluate(interp).getNumber();
      graphics().hplot(x, y);

      for (size_t i = 1; i < coords_.size(); ++i) {
        x = coords_[i].first->evaluate(interp).getNumber();
        y = coords_[i].second->evaluate(interp).getNumber();
        graphics().hplot_to(x, y);
      }
    }
  }

private:
  std::vector<
      std::pair<std::shared_ptr<Expression>, std::shared_ptr<Expression>>>
      coords_;
};

class MoveStmt : public Statement {
public:
  MoveStmt(std::shared_ptr<Expression> x, std::shared_ptr<Expression> y)
      : x_(std::move(x)), y_(std::move(y)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    graphics().move(x_->evaluate(interp).getNumber(),
                    y_->evaluate(interp).getNumber());
  }

private:
  std::shared_ptr<Expression> x_;
  std::shared_ptr<Expression> y_;
};

class RotateStmt : public Statement {
public:
  explicit RotateStmt(std::shared_ptr<Expression> angle)
      : angle_(std::move(angle)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    int angle = static_cast<int>(angle_->evaluate(interp).getNumber());
    graphics().setRotate(angle);
  }

private:
  std::shared_ptr<Expression> angle_;
};

class ScaleStmt : public Statement {
public:
  explicit ScaleStmt(std::shared_ptr<Expression> scale)
      : scale_(std::move(scale)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    int s = static_cast<int>(scale_->evaluate(interp).getNumber());
    graphics().setScale(s);
  }

private:
  std::shared_ptr<Expression> scale_;
};

class ShloadStmt : public Statement {
public:
  explicit ShloadStmt(const std::string &filename = "")
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    if (!filename_.empty()) {
      // Load from binary file
      interp->loadShapeTableFromFile(filename_);
    } else {
      // Load from DATA statements (existing behavior)
      // Read shape number
      Value shapeNumVal = interp->readData();
      int shapeNum = static_cast<int>(shapeNumVal.getNumber());

      // Read number of points
      Value numPointsVal = interp->readData();
      int numPoints = static_cast<int>(numPointsVal.getNumber());

      // Read point pairs
      std::vector<std::pair<double, double>> points;
      points.reserve(static_cast<size_t>(numPoints));
      for (int i = 0; i < numPoints; ++i) {
        Value xVal = interp->readData();
        Value yVal = interp->readData();
        points.push_back({xVal.getNumber(), yVal.getNumber()});
      }

      // Load shape into graphics
      graphics().loadShape(shapeNum, points);
    }
  }

private:
  std::string filename_;
};

class DrawStmt : public Statement {
public:
  DrawStmt(std::shared_ptr<Expression> shape,
           std::shared_ptr<Expression> x = nullptr,
           std::shared_ptr<Expression> y = nullptr)
      : shape_(std::move(shape)), x_(std::move(x)), y_(std::move(y)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    int shapeNum = static_cast<int>(shape_->evaluate(interp).getNumber());
    if (x_ && y_) {
      double xVal = x_->evaluate(interp).getNumber();
      double yVal = y_->evaluate(interp).getNumber();
      graphics().draw(shapeNum, xVal, yVal);
    } else {
      graphics().draw(shapeNum);
    }
  }

private:
  std::shared_ptr<Expression> shape_;
  std::shared_ptr<Expression> x_;
  std::shared_ptr<Expression> y_;
};

class XdrawStmt : public Statement {
public:
  XdrawStmt(std::shared_ptr<Expression> shape,
            std::shared_ptr<Expression> x = nullptr,
            std::shared_ptr<Expression> y = nullptr)
      : shape_(std::move(shape)), x_(std::move(x)), y_(std::move(y)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    int shapeNum = static_cast<int>(shape_->evaluate(interp).getNumber());
    if (x_ && y_) {
      double xVal = x_->evaluate(interp).getNumber();
      double yVal = y_->evaluate(interp).getNumber();
      graphics().xdraw(shapeNum, xVal, yVal);
    } else {
      graphics().xdraw(shapeNum);
    }
  }

private:
  std::shared_ptr<Expression> shape_;
  std::shared_ptr<Expression> x_;
  std::shared_ptr<Expression> y_;
};

class OnTransferStmt : public Statement {
public:
  enum Kind { Goto, Gosub };
  OnTransferStmt(std::shared_ptr<Expression> index, Kind kind,
                 std::vector<int> lines)
      : index_(std::move(index)), kind_(kind), lines_(std::move(lines)) {}
  void execute(Interpreter *interp) override {
    int n = static_cast<int>(index_->evaluate(interp).getNumber());
    if (n < 1 || n > static_cast<int>(lines_.size())) {
      return; // Do nothing if out of range (Applesoft behavior)
    }
    int line = lines_[static_cast<size_t>(n - 1)];
    if (kind_ == Goto) {
      interp->gotoLine(line);
    } else {
      interp->gosub(line);
    }
  }

private:
  std::shared_ptr<Expression> index_;
  Kind kind_;
  std::vector<int> lines_;
};

class HtabStmt : public Statement {
public:
  explicit HtabStmt(std::shared_ptr<Expression> col) : col_(std::move(col)) {}
  void execute(Interpreter *interp) override {
    int target = static_cast<int>(col_->evaluate(interp).getNumber());
    interp->htab(target);
  }

private:
  std::shared_ptr<Expression> col_;
};

class VtabStmt : public Statement {
public:
  explicit VtabStmt(std::shared_ptr<Expression> row) : row_(std::move(row)) {}
  void execute(Interpreter *interp) override {
    int target = static_cast<int>(row_->evaluate(interp).getNumber());
    interp->vtab(target);
  }

private:
  std::shared_ptr<Expression> row_;
};

class InverseStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->setInverse(true); }
};

class NormalStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->setNormal(); }
};

class FlashStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->setFlash(true); }
};

class HgrStmt : public Statement {
public:
  void execute(Interpreter *interp) override { 
    interp->requireGraphicsMode();
    graphics().enterHighRes(); 
  }
};

class HcolorStmt : public Statement {
public:
  explicit HcolorStmt(std::shared_ptr<Expression> color)
      : color_(std::move(color)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    int c = static_cast<int>(color_->evaluate(interp).getNumber());
    graphics().setColor(c);
  }

private:
  std::shared_ptr<Expression> color_;
};

class ColorStmt : public Statement {
public:
  explicit ColorStmt(std::shared_ptr<Expression> color)
      : color_(std::move(color)) {}
  void execute(Interpreter *interp) override {
    interp->requireGraphicsMode();
    int c = static_cast<int>(color_->evaluate(interp).getNumber());
    graphics().setColor(c);
  }

private:
  std::shared_ptr<Expression> color_;
};

// Expression classes
class LiteralExpr : public Expression {
public:
  explicit LiteralExpr(const Value &val) : value_(val) {}
  Value evaluate(Interpreter *) override { return value_; }

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

class NotExpr : public Expression {
public:
  explicit NotExpr(std::shared_ptr<Expression> operand)
      : operand_(std::move(operand)) {}

  Value evaluate(Interpreter *interp) override {
    double v = operand_->evaluate(interp).getNumber();
    return Value(v == 0.0 ? 1.0 : 0.0);
  }

private:
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
    case TokenType::SCRN:
      return funcScrn(argValues[0], argValues[1]);
    case TokenType::USR:
      return funcUsr(argValues[0]);
    case TokenType::PEEK:
      return funcPeek(argValues[0]);
    case TokenType::FRE:
      return funcFre(argValues[0]);
    case TokenType::PDL:
      return funcPdl(argValues[0]);
    case TokenType::POS:
      return funcPos(argValues[0]);
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
  enum class Separator { None, Semicolon, Comma };

  PrintStmt(std::vector<std::shared_ptr<Expression>> exprs,
            std::vector<Separator> separators)
      : exprs_(std::move(exprs)), separators_(std::move(separators)) {}

  void execute(Interpreter *interp) override {
    if (exprs_.empty()) {
      interp->printNewline();
      return;
    }

    for (size_t i = 0; i < exprs_.size(); ++i) {
      Value val = exprs_[i]->evaluate(interp);
      interp->printText(val.getString());

      Separator sep = i < separators_.size() ? separators_[i] : Separator::None;
      switch (sep) {
      case Separator::Comma:
        interp->printToNextZone();
        break;
      case Separator::Semicolon:
        break;
      case Separator::None:
        interp->printNewline();
        break;
      }
    }
  }

private:
  std::vector<std::shared_ptr<Expression>> exprs_;
  std::vector<Separator> separators_;
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
  explicit RestoreStmt(std::shared_ptr<Expression> target)
      : target_(std::move(target)) {}

  void execute(Interpreter *interp) override {
    int line = -1;
    if (target_) {
      line = static_cast<int>(target_->evaluate(interp).getNumber());
    }
    interp->restoreData(line);
  }

private:
  std::shared_ptr<Expression> target_;
};

class PokeStmt : public Statement {
public:
  PokeStmt(std::shared_ptr<Expression> addr, std::shared_ptr<Expression> val)
      : addr_(std::move(addr)), val_(std::move(val)) {}

  void execute(Interpreter *interp) override {
    int a = static_cast<int>(addr_->evaluate(interp).getNumber());
    int v = static_cast<int>(val_->evaluate(interp).getNumber());
    pokeMemory(a, v);
  }

private:
  std::shared_ptr<Expression> addr_;
  std::shared_ptr<Expression> val_;
};

class CallStmt : public Statement {
public:
  explicit CallStmt(std::shared_ptr<Expression> addr)
      : addr_(std::move(addr)) {}

  void execute(Interpreter *interp) override {
    int address = static_cast<int>(addr_->evaluate(interp).getNumber());
    interp->callAddress(address);
  }

private:
  std::shared_ptr<Expression> addr_;
};

class HomeStmt : public Statement {
public:
  void execute(Interpreter *) override { std::cout << "\x1b[2J\x1b[H"; }
};

class TextStmt : public Statement {
public:
  // TEXT switches to text mode (no graphics)
  // In original Applesoft BASIC, TEXT would switch the display to text mode.
  // In this implementation, text mode is the default and graphics are
  // offscreen-only, so TEXT is effectively a no-op that confirms text mode.
  void execute(Interpreter *) override {}
};

class GrStmt : public Statement {
public:
  void execute(Interpreter *interp) override { 
    interp->requireGraphicsMode();
    graphics().enterLowRes(); 
  }
};

class HiresStmt : public Statement {
public:
  void execute(Interpreter *interp) override { 
    interp->requireGraphicsMode();
    graphics().enterHighRes(); 
  }
};

class ClrStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->clearState(); }
};

class GetStmt : public Statement {
public:
  explicit GetStmt(std::string name) : name_(std::move(name)) {}

  void execute(Interpreter *interp) override {
    char ch = '\0';
    if (!std::cin.get(ch)) {
      ch = '\0';
    }

    if (!name_.empty() && name_.back() == '$') {
      interp->getVariables().setVariable(name_, Value(std::string(1, ch)));
    } else {
      interp->getVariables().setVariable(
          name_, Value(static_cast<double>(static_cast<unsigned char>(ch))));
    }
  }

private:
  std::string name_;
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

class TraceStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->setTrace(true); }
};

class NoTraceStmt : public Statement {
public:
  void execute(Interpreter *interp) override { interp->setTrace(false); }
};

class RandomizeStmt : public Statement {
public:
  explicit RandomizeStmt(std::shared_ptr<Expression> seed)
      : seed_(std::move(seed)) {}
  void execute(Interpreter *interp) override {
    double s = seed_ ? seed_->evaluate(interp).getNumber() : 1.0;
    interp->randomize(s);
  }

private:
  std::shared_ptr<Expression> seed_;
};

class SpeedStmt : public Statement {
public:
  explicit SpeedStmt(std::shared_ptr<Expression> delay)
      : delay_(std::move(delay)) {}
  void execute(Interpreter *interp) override {
    int raw = static_cast<int>(delay_->evaluate(interp).getNumber());
    if (raw < 0)
      raw = 0;
    if (raw > 255)
      raw = 255;
    interp->setSpeedDelay(raw);
  }

private:
  std::shared_ptr<Expression> delay_;
};

class PrStmt : public Statement {
public:
  explicit PrStmt(std::shared_ptr<Expression> slot) : slot_(std::move(slot)) {}
  void execute(Interpreter *interp) override {
    int device = static_cast<int>(slot_->evaluate(interp).getNumber());
    if (device < 0)
      device = 0;
    interp->setOutputDevice(device);
  }

private:
  std::shared_ptr<Expression> slot_;
};

class InStmt : public Statement {
public:
  explicit InStmt(std::shared_ptr<Expression> slot) : slot_(std::move(slot)) {}
  void execute(Interpreter *interp) override {
    int device = static_cast<int>(slot_->evaluate(interp).getNumber());
    if (device < 0)
      device = 0;
    interp->setInputDevice(device);
  }

private:
  std::shared_ptr<Expression> slot_;
};

class WhileStmt : public Statement {
public:
  WhileStmt(std::shared_ptr<Expression> condition, LineNumber returnLine)
      : condition_(std::move(condition)), returnLine_(returnLine) {}
  void execute(Interpreter *interp) override;

private:
  std::shared_ptr<Expression> condition_;
  LineNumber returnLine_;
};

class WendStmt : public Statement {
public:
  void execute(Interpreter *interp) override;
};

class PopStmt : public Statement {
public:
  void execute(Interpreter *interp) override;
};

class WaitStmt : public Statement {
public:
  WaitStmt(std::shared_ptr<Expression> addr, std::shared_ptr<Expression> mask,
           std::shared_ptr<Expression> timeout = nullptr)
      : addr_(std::move(addr)), mask_(std::move(mask)),
        timeout_(std::move(timeout)) {}
  void execute(Interpreter *interp) override {
    int a = static_cast<int>(addr_->evaluate(interp).getNumber());
    int m = static_cast<int>(mask_->evaluate(interp).getNumber());
    // Optional timeout in milliseconds; <=0 means no timeout
    int timeoutMs = 0;
    if (timeout_) {
      timeoutMs = static_cast<int>(timeout_->evaluate(interp).getNumber());
      if (timeoutMs < 0)
        timeoutMs = 0;
    }

    auto start = std::chrono::steady_clock::now();
    while (true) {
      int val = peekMemory(a);
      if ((val & m) != 0)
        break;
      if (timeoutMs > 0) {
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
        if (elapsed.count() >= timeoutMs) {
          // Timeout expires silently (Applesoft allowed external
          // events/timeouts)
          break;
        }
      }
      // Yield to avoid busy-loop; in real Applesoft would poll I/O
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
  }

private:
  std::shared_ptr<Expression> addr_;
  std::shared_ptr<Expression> mask_;
  std::shared_ptr<Expression> timeout_;
};

class HimemStmt : public Statement {
public:
  explicit HimemStmt(std::shared_ptr<Expression> addr)
      : addr_(std::move(addr)) {}
  void execute(Interpreter *interp) override {
    int val = static_cast<int>(addr_->evaluate(interp).getNumber());
    interp->setHimem(val);
  }

private:
  std::shared_ptr<Expression> addr_;
};

class LomemStmt : public Statement {
public:
  explicit LomemStmt(std::shared_ptr<Expression> addr)
      : addr_(std::move(addr)) {}
  void execute(Interpreter *interp) override {
    int val = static_cast<int>(addr_->evaluate(interp).getNumber());
    interp->setLomem(val);
  }

private:
  std::shared_ptr<Expression> addr_;
};

class RecallStmt : public Statement {
public:
  explicit RecallStmt(const std::string &arrayName)
      : arrayName_(arrayName) {}
  void execute(Interpreter *interp) override {
    interp->recallArray(arrayName_);
  }

private:
  std::string arrayName_;
};

class StoreStmt : public Statement {
public:
  explicit StoreStmt(const std::string &arrayName)
      : arrayName_(arrayName) {}
  void execute(Interpreter *interp) override {
    interp->storeArray(arrayName_);
  }

private:
  std::string arrayName_;
};

class TapeStmt : public Statement {
public:
  explicit TapeStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    if (filename_.empty()) {
      // Show current tape
      std::string current = interp->getTapeFile();
      if (current.empty()) {
        std::cout << "NO TAPE LOADED\n";
      } else {
        std::cout << "CURRENT TAPE: " << current << "\n";
      }
    } else {
      // Set tape file
      interp->setTapeFile(filename_);
      std::cout << "TAPE SET TO: " << filename_ << "\n";
    }
  }

private:
  std::string filename_;
};

// ProDOS file operation statements
class OpenFileStmt : public Statement {
public:
  explicit OpenFileStmt(const std::string &filename, const std::string &options)
      : filename_(filename), options_(options) {}
  void execute(Interpreter *interp) override {
    interp->openFile(filename_, options_);
  }

private:
  std::string filename_;
  std::string options_;
};

class CloseFileStmt : public Statement {
public:
  explicit CloseFileStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    if (filename_.empty()) {
      interp->closeAllFiles();
    } else {
      interp->closeFile(filename_);
    }
  }

private:
  std::string filename_;
};

class AppendFileStmt : public Statement {
public:
  explicit AppendFileStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->appendFile(filename_);
  }

private:
  std::string filename_;
};

class FlushFileStmt : public Statement {
public:
  explicit FlushFileStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->flushFile(filename_);
  }

private:
  std::string filename_;
};

class CreateFileStmt : public Statement {
public:
  explicit CreateFileStmt(const std::string &filename, const std::string &options)
      : filename_(filename), options_(options) {}
  void execute(Interpreter *interp) override {
    interp->createFile(filename_, options_);
  }

private:
  std::string filename_;
  std::string options_;
};

class LockFileStmt : public Statement {
public:
  explicit LockFileStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->lockFile(filename_);
  }

private:
  std::string filename_;
};

class UnlockFileStmt : public Statement {
public:
  explicit UnlockFileStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->unlockFile(filename_);
  }

private:
  std::string filename_;
};

class BloadFileStmt : public Statement {
public:
  explicit BloadFileStmt(const std::string &filename, int address)
      : filename_(filename), address_(address) {}
  void execute(Interpreter *interp) override {
    interp->bloadFile(filename_, address_);
  }

private:
  std::string filename_;
  int address_;
};

class BsaveFileStmt : public Statement {
public:
  explicit BsaveFileStmt(const std::string &filename, int address, int length)
      : filename_(filename), address_(address), length_(length) {}
  void execute(Interpreter *interp) override {
    interp->bsaveFile(filename_, address_, length_);
  }

private:
  std::string filename_;
  int address_;
  int length_;
};

class BrunFileStmt : public Statement {
public:
  explicit BrunFileStmt(const std::string &filename, int address)
      : filename_(filename), address_(address) {}
  void execute(Interpreter *interp) override {
    interp->brunFile(filename_, address_);
  }

private:
  std::string filename_;
  int address_;
};

class ProdosStoreStmt : public Statement {
public:
  explicit ProdosStoreStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->storeVariables(filename_);
  }

private:
  std::string filename_;
};

class ProdosRestoreStmt : public Statement {
public:
  explicit ProdosRestoreStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->restoreVariables(filename_);
  }

private:
  std::string filename_;
};

class DeleteFileStmt : public Statement {
public:
  explicit DeleteFileStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->deleteFile(filename_);
  }

private:
  std::string filename_;
};

class RenameFileStmt : public Statement {
public:
  RenameFileStmt(const std::string &oldName, const std::string &newName)
      : oldName_(oldName), newName_(newName) {}
  void execute(Interpreter *interp) override {
    interp->renameFile(oldName_, newName_);
  }

private:
  std::string oldName_;
  std::string newName_;
};

class PrefixStmt : public Statement {
public:
  explicit PrefixStmt(const std::string &path)
      : path_(path) {}
  void execute(Interpreter *interp) override {
    interp->setPrefix(path_);
  }

private:
  std::string path_;
};

class PositionFileStmt : public Statement {
public:
  PositionFileStmt(const std::string &filename, int record, int byte)
      : filename_(filename), record_(record), byte_(byte) {}
  void execute(Interpreter *interp) override {
    interp->positionFile(filename_, record_, byte_);
  }

private:
  std::string filename_;
  int record_;
  int byte_;
};

class ChainStmt : public Statement {
public:
  ChainStmt(const std::string &filename, int startLine)
      : filename_(filename), startLine_(startLine) {}
  void execute(Interpreter *interp) override {
    interp->chainProgram(filename_, startLine_);
  }

private:
  std::string filename_;
  int startLine_;
};

class ExecStmt : public Statement {
public:
  explicit ExecStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->execFile(filename_);
  }

private:
  std::string filename_;
};

class DashStmt : public Statement {
public:
  explicit DashStmt(const std::string &filename)
      : filename_(filename) {}
  void execute(Interpreter *interp) override {
    interp->dashRun(filename_);
  }

private:
  std::string filename_;
};

class CatStmt : public Statement {
public:
  explicit CatStmt(const std::string &path)
      : path_(path) {}
  void execute(Interpreter *interp) override {
    interp->catalogFiles(path_);
  }

private:
  std::string path_;
};

class ProdosReadStmt : public Statement {
public:
  ProdosReadStmt(const std::string &filename, int record, int byte)
      : filename_(filename), record_(record), byte_(byte) {}
  void execute(Interpreter *interp) override {
    interp->readFile(filename_, record_, byte_);
  }

private:
  std::string filename_;
  int record_;
  int byte_;
};

class ProdosWriteStmt : public Statement {
public:
  ProdosWriteStmt(const std::string &filename, int record)
      : filename_(filename), record_(record) {}
  void execute(Interpreter *interp) override {
    interp->writeFile(filename_, record_);
  }

private:
  std::string filename_;
  int record_;
};

/**
 * @brief WHILE statement execution - pushes loop state onto stack
 * 
 * Begins a WHILE loop by evaluating the condition and pushing loop state.
 * The actual loop logic (condition checking, iteration) is handled by the
 * interpreter's WHILE stack management.
 * 
 * BASIC Usage:
 *   WHILE condition
 *     ... statements ...
 *   WEND
 * 
 * @param interp Interpreter instance
 */
void WhileStmt::execute(Interpreter *interp) {
  interp->pushWhileLoop(condition_, interp->getCurrentLine());
}

/**
 * @brief WEND statement execution - loops back to matching WHILE
 * 
 * Marks the end of a WHILE loop and jumps back to the corresponding WHILE
 * statement for condition re-evaluation. The interpreter handles loop
 * stack management and condition checking.
 * 
 * @param interp Interpreter instance
 */
void WendStmt::execute(Interpreter *interp) { interp->nextWhileLoop(); }

/**
 * @brief POP statement execution - removes GOSUB return address from stack
 * 
 * Removes the most recent GOSUB return address from the stack without
 * returning. This is useful for breaking out of subroutines when you want
 * to jump directly to another location instead of returning to the caller.
 * 
 * BASIC Usage:
 *   100 GOSUB 1000
 *   110 END
 *   1000 REM Subroutine
 *   1010 IF X > 0 THEN POP : GOTO 2000
 *   1020 RETURN
 *   2000 REM Jump here instead of returning to line 110
 * 
 * @param interp Interpreter instance
 */
void PopStmt::execute(Interpreter *interp) { interp->popGosub(); }

// Parser implementation
/**
 * @brief Construct a new Parser object
 * 
 * Initializes an empty parser. The parser is stateless and can be reused
 * for parsing multiple token sequences.
 */
Parser::Parser() {}

/**
 * @brief Parse a token sequence into statement AST nodes
 * 
 * This is the main entry point for parsing. It converts a flat sequence of
 * tokens (from the tokenizer) into a structured sequence of Statement objects
 * that can be executed by the interpreter.
 * 
 * Parsing process:
 * 1. Skip newlines and empty tokens
 * 2. Handle statement separators (colons) for multi-statement lines
 * 3. Dispatch to specific statement parsers based on first token
 * 4. Build statement objects and add to result vector
 * 
 * Multi-statement lines:
 * Applesoft BASIC allows multiple statements on one line separated by colons:
 *   10 PRINT "A": X = 5: PRINT X
 * This function handles the colon separation and parses each statement
 * independently.
 * 
 * Error handling:
 * - Invalid syntax throws std::runtime_error from individual parsers
 * - Unexpected tokens are reported as syntax errors
 * - Position tracking ensures error messages can identify problem location
 * 
 * @param tokens Token sequence from tokenizer
 * @return Vector of parsed Statement objects ready for execution
 * @throws std::runtime_error on syntax errors
 */
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

    // Check for statement separator (colon)
    // Multiple statements on one line: 10 PRINT "A": PRINT "B"
    if (tokens[pos].type == TokenType::COLON) {
      pos++;
      continue;
    }

    // Parse next statement and add to result
    auto stmt = parseStatement(tokens, pos);
    if (stmt) {
      statements.push_back(stmt);
    }
  }

  return statements;
}

/**
 * @brief Parse a single statement from token sequence
 * 
 * Dispatches to specific statement parsers based on the first token type.
 * This is the central dispatch point for all BASIC statement types.
 * 
 * Statement categories:
 * - I/O: PRINT, INPUT, GET
 * - Assignment: LET, variable = expression
 * - Control flow: IF/THEN/ELSE, FOR/NEXT, WHILE/WEND, GOTO, GOSUB, RETURN
 * - Data: DATA, READ, RESTORE, DIM
 * - Graphics: GR, HGR, HPLOT, COLOR, DRAW, etc.
 * - System: NEW, END, STOP, CLR, HOME, TEXT
 * - Functions: DEF FN
 * - Error handling: ONERR, RESUME
 * - File I/O: LOAD, SAVE, etc.
 * 
 * Parsing strategy:
 * - Each statement type has a dedicated parse function (parseIf, parsePrint, etc.)
 * - Parser functions consume tokens and advance the position reference
 * - Return nullptr if no valid statement found at current position
 * - Throw exception on syntax errors
 * 
 * @param tokens Token sequence
 * @param pos Current position in token sequence (updated by parser)
 * @return Parsed Statement object, or nullptr if no statement found
 * @throws std::runtime_error on syntax errors
 */
std::shared_ptr<Statement>
Parser::parseStatement(const std::vector<Token> &tokens, size_t &pos) {
  if (pos >= tokens.size())
    return nullptr;

  Token &token = const_cast<Token &>(tokens[pos]);

  // Dispatch to appropriate statement parser based on token type
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
  case TokenType::STOP:
    pos++;
    return std::make_shared<StopStmt>();
  case TokenType::HTAB: {
    pos++; // Skip HTAB
    auto col = parseExpression(tokens, pos);
    return std::make_shared<HtabStmt>(col);
  }
  case TokenType::VTAB: {
    pos++; // Skip VTAB
    auto row = parseExpression(tokens, pos);
    return std::make_shared<VtabStmt>(row);
  }
  case TokenType::INVERSE:
    pos++;
    return std::make_shared<InverseStmt>();
  case TokenType::NORMAL:
    pos++;
    return std::make_shared<NormalStmt>();
  case TokenType::FLASH:
    pos++;
    return std::make_shared<FlashStmt>();
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
  case TokenType::RESTORE: {
    pos++; // Skip RESTORE
    // Check if it's ProDOS RESTORE (followed by string filename) or DATA RESTORE
    if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
      // ProDOS RESTORE pn
      std::string filename = tokens[pos].value.getString();
      pos++;
      // TODO: Parse optional S# and D# parameters (slot and drive)
      return std::make_shared<ProdosRestoreStmt>(filename);
    } else {
      // DATA RESTORE [line_number]
      std::shared_ptr<Expression> target;
      if (pos < tokens.size() && tokens[pos].type != TokenType::COLON &&
          tokens[pos].type != TokenType::NEWLINE) {
        target = parseExpression(tokens, pos);
      }
      return std::make_shared<RestoreStmt>(target);
    }
  }
  case TokenType::DEF:
    return parseDef(tokens, pos);
  case TokenType::ON:
    return parseOn(tokens, pos);
  case TokenType::POKE: {
    pos++; // Skip POKE
    auto addr = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto val = parseExpression(tokens, pos);
    return std::make_shared<PokeStmt>(addr, val);
  }
  case TokenType::CALL: {
    pos++; // Skip CALL
    auto addr = parseExpression(tokens, pos);
    return std::make_shared<CallStmt>(addr);
  }
  case TokenType::HOME:
    pos++;
    return std::make_shared<HomeStmt>();
  case TokenType::TEXT:
    pos++;
    return std::make_shared<TextStmt>();
  case TokenType::CLR:
    pos++;
    return std::make_shared<ClrStmt>();
  case TokenType::GR:
    pos++;
    return std::make_shared<GrStmt>();
  case TokenType::HIRES:
    pos++;
    return std::make_shared<HiresStmt>();
  case TokenType::HGR:
    pos++;
    return std::make_shared<HgrStmt>();
  case TokenType::HGR2:
    pos++;
    return std::make_shared<HgrStmt>();
  case TokenType::HCOLOR: {
    pos++; // Skip HCOLOR=
    auto color = parseExpression(tokens, pos);
    return std::make_shared<HcolorStmt>(color);
  }
  case TokenType::COLOR: {
    pos++; // Skip COLOR=
    auto color = parseExpression(tokens, pos);
    return std::make_shared<ColorStmt>(color);
  }
  case TokenType::PLOT: {
    pos++; // Skip PLOT
    auto x = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto y = parseExpression(tokens, pos);
    return std::make_shared<PlotStmt>(x, y);
  }
  case TokenType::HLIN: {
    pos++; // Skip HLIN
    auto x1 = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto x2 = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto y = parseExpression(tokens, pos);
    return std::make_shared<HlinStmt>(x1, x2, y);
  }
  case TokenType::VLIN: {
    pos++; // Skip VLIN
    auto y1 = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto y2 = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto x = parseExpression(tokens, pos);
    return std::make_shared<VlinStmt>(y1, y2, x);
  }
  case TokenType::HPLOT: {
    pos++; // Skip HPLOT
    std::vector<
        std::pair<std::shared_ptr<Expression>, std::shared_ptr<Expression>>>
        coords;
    auto x = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto y = parseExpression(tokens, pos);
    coords.push_back({x, y});

    // Handle additional coordinate pairs separated by commas or TO
    while (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      // Check for TO keyword for line drawing
      if (pos < tokens.size() && tokens[pos].type == TokenType::TO) {
        pos++; // Skip TO
      }
      x = parseExpression(tokens, pos);
      if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
        throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
      }
      pos++;
      y = parseExpression(tokens, pos);
      coords.push_back({x, y});
    }
    return std::make_shared<HplotStmt>(coords);
  }
  case TokenType::MOVE: {
    pos++; // Skip MOVE
    auto x = parseExpression(tokens, pos);
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++;
    auto y = parseExpression(tokens, pos);
    return std::make_shared<MoveStmt>(x, y);
  }
  case TokenType::ROTATE: {
    pos++; // Skip ROTATE
    auto angle = parseExpression(tokens, pos);
    return std::make_shared<RotateStmt>(angle);
  }
  case TokenType::SCALE: {
    pos++; // Skip SCALE
    auto scale = parseExpression(tokens, pos);
    return std::make_shared<ScaleStmt>(scale);
  }
  case TokenType::SHLOAD:
    return parseShload(tokens, pos);
  case TokenType::DRAW: {
    pos++; // Skip DRAW
    auto shapeNum = parseExpression(tokens, pos);
    std::shared_ptr<Expression> x = nullptr;
    std::shared_ptr<Expression> y = nullptr;

    // Check for AT clause
    if (pos < tokens.size() && tokens[pos].type == TokenType::AT) {
      pos++; // Skip AT
      x = parseExpression(tokens, pos);
      if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
        throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
      }
      pos++;
      y = parseExpression(tokens, pos);
    }

    return std::make_shared<DrawStmt>(shapeNum, x, y);
  }
  case TokenType::XDRAW: {
    pos++; // Skip XDRAW
    auto shapeNum = parseExpression(tokens, pos);
    std::shared_ptr<Expression> x = nullptr;
    std::shared_ptr<Expression> y = nullptr;

    // Check for AT clause
    if (pos < tokens.size() && tokens[pos].type == TokenType::AT) {
      pos++; // Skip AT
      x = parseExpression(tokens, pos);
      if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
        throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
      }
      pos++;
      y = parseExpression(tokens, pos);
    }

    return std::make_shared<XdrawStmt>(shapeNum, x, y);
  }
  case TokenType::GET: {
    pos++; // Skip GET
    if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED VARIABLE");
    }
    std::string varName = tokens[pos].text;
    pos++;
    return std::make_shared<GetStmt>(varName);
  }
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
  case TokenType::TRACE:
    pos++;
    return std::make_shared<TraceStmt>();
  case TokenType::NOTRACE:
    pos++;
    return std::make_shared<NoTraceStmt>();
  case TokenType::RANDOMIZE:
    return parseRandomize(tokens, pos);
  case TokenType::SPEED:
    return parseSpeed(tokens, pos);
  case TokenType::PR:
    return parseDeviceRedirect(tokens, pos, true);
  case TokenType::IN:
    return parseDeviceRedirect(tokens, pos, false);
  case TokenType::WHILE:
    return parseWhile(tokens, pos);
  case TokenType::WEND:
    pos++;
    return std::make_shared<WendStmt>();
  case TokenType::POP:
    pos++;
    return std::make_shared<PopStmt>();
  case TokenType::WAIT:
    return parseWait(tokens, pos);
  case TokenType::HIMEM:
    return parseHimem(tokens, pos);
  case TokenType::LOMEM:
    return parseLomem(tokens, pos);
  case TokenType::RECALL: {
    pos++; // Skip RECALL
    if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED ARRAY NAME");
    }
    std::string arrayName = tokens[pos].text;
    pos++;
    return std::make_shared<RecallStmt>(arrayName);
  }
  case TokenType::STORE: {
    pos++; // Skip STORE
    // Check if it's ProDOS STORE (followed by string filename) or array STORE
    if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
      // ProDOS STORE pn
      std::string filename = tokens[pos].value.getString();
      pos++;
      // TODO: Parse optional S# and D# parameters (slot and drive)
      return std::make_shared<ProdosStoreStmt>(filename);
    } else {
      // Array STORE arrayname
      if (pos >= tokens.size() || tokens[pos].type != TokenType::IDENTIFIER) {
        throw std::runtime_error("SYNTAX ERROR: EXPECTED ARRAY NAME OR FILENAME");
      }
      std::string arrayName = tokens[pos].text;
      pos++;
      return std::make_shared<StoreStmt>(arrayName);
    }
  }
  case TokenType::TAPE: {
    pos++; // Skip TAPE
    if (pos >= tokens.size()) {
      // TAPE with no arguments - show current tape
      return std::make_shared<TapeStmt>("");
    }
    if (tokens[pos].type == TokenType::STRING) {
      // TAPE "filename" - set tape file
      std::string filename = tokens[pos].value.getString();
      pos++;
      return std::make_shared<TapeStmt>(filename);
    }
    // TAPE with no arguments - show current tape
    return std::make_shared<TapeStmt>("");
  }
  case TokenType::OPEN: {
    pos++; // Skip OPEN
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    // Parse optional options (not fully implemented yet)
    return std::make_shared<OpenFileStmt>(filename, "");
  }
  case TokenType::CLOSE: {
    pos++; // Skip CLOSE
    std::string filename = "";
    if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
      filename = tokens[pos].value.getString();
      pos++;
    }
    return std::make_shared<CloseFileStmt>(filename);
  }
  case TokenType::APPEND: {
    pos++; // Skip APPEND
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    return std::make_shared<AppendFileStmt>(filename);
  }
  case TokenType::FLUSH: {
    pos++; // Skip FLUSH
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    return std::make_shared<FlushFileStmt>(filename);
  }
  case TokenType::CREATE: {
    pos++; // Skip CREATE
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    return std::make_shared<CreateFileStmt>(filename, "");
  }
  case TokenType::LOCK: {
    pos++; // Skip LOCK
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    return std::make_shared<LockFileStmt>(filename);
  }
  case TokenType::UNLOCK: {
    pos++; // Skip UNLOCK
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    return std::make_shared<UnlockFileStmt>(filename);
  }
  case TokenType::BLOAD: {
    pos++; // Skip BLOAD
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int address = -1;
    // Parse optional ,A# parameter
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      if (pos < tokens.size()) {
        // Could be A# or just a number
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'A') {
          std::string addrStr = tokens[pos].text.substr(1);
          address = std::stoi(addrStr);
          pos++;
        } else if (tokens[pos].type == TokenType::NUMBER) {
          address = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
      }
    }
    return std::make_shared<BloadFileStmt>(filename, address);
  }
  case TokenType::BSAVE: {
    pos++; // Skip BSAVE
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int address = 0, length = 0;
    // Parse ,A#,L# parameters
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip first comma
      if (pos < tokens.size()) {
        // Parse A# or just a number
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'A') {
          std::string addrStr = tokens[pos].text.substr(1);
          address = std::stoi(addrStr);
          pos++;
        } else if (tokens[pos].type == TokenType::NUMBER) {
          address = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
        
        // Parse ,L# parameter
        if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
          pos++; // Skip second comma
          if (pos < tokens.size()) {
            if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'L') {
              std::string lenStr = tokens[pos].text.substr(1);
              length = std::stoi(lenStr);
              pos++;
            } else if (tokens[pos].type == TokenType::NUMBER) {
              length = static_cast<int>(tokens[pos].value.getNumber());
              pos++;
            }
          }
        }
      }
    }
    return std::make_shared<BsaveFileStmt>(filename, address, length);
  }
  case TokenType::BRUN: {
    pos++; // Skip BRUN
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int address = -1;
    // Parse optional ,A# parameter
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      if (pos < tokens.size()) {
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'A') {
          std::string addrStr = tokens[pos].text.substr(1);
          address = std::stoi(addrStr);
          pos++;
        } else if (tokens[pos].type == TokenType::NUMBER) {
          address = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
      }
    }
    return std::make_shared<BrunFileStmt>(filename, address);
  }
  case TokenType::DELETE: {
    pos++; // Skip DELETE
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<DeleteFileStmt>(filename);
  }
  case TokenType::RENAME: {
    pos++; // Skip RENAME
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string oldName = tokens[pos].value.getString();
    pos++;
    if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
    }
    pos++; // Skip comma
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED NEW FILENAME");
    }
    std::string newName = tokens[pos].value.getString();
    pos++;
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<RenameFileStmt>(oldName, newName);
  }
  case TokenType::PREFIX: {
    pos++; // Skip PREFIX
    std::string path = "";
    if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
      path = tokens[pos].value.getString();
      pos++;
    }
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<PrefixStmt>(path);
  }
  case TokenType::POSITION: {
    pos++; // Skip POSITION
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int record = 0, byte = 0;
    // Parse optional ,Rrecord# parameter
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      if (pos < tokens.size()) {
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'R') {
          try {
            std::string recStr = tokens[pos].text.substr(1);
            record = std::stoi(recStr);
            pos++;
          } catch (...) {
            throw std::runtime_error("SYNTAX ERROR: INVALID RECORD NUMBER");
          }
        } else if (tokens[pos].type == TokenType::NUMBER) {
          record = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
      }
      // Parse optional ,Bbyte# parameter
      if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
        pos++; // Skip comma
        if (pos < tokens.size()) {
          if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'B') {
            try {
              std::string byteStr = tokens[pos].text.substr(1);
              byte = std::stoi(byteStr);
              pos++;
            } catch (...) {
              throw std::runtime_error("SYNTAX ERROR: INVALID BYTE NUMBER");
            }
          } else if (tokens[pos].type == TokenType::NUMBER) {
            byte = static_cast<int>(tokens[pos].value.getNumber());
            pos++;
          }
        }
      }
    }
    return std::make_shared<PositionFileStmt>(filename, record, byte);
  }
  case TokenType::CHAIN: {
    pos++; // Skip CHAIN
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int startLine = -1;
    // Parse optional ,@# parameter (starting line number)
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      if (pos < tokens.size()) {
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == '@') {
          try {
            std::string lineStr = tokens[pos].text.substr(1);
            startLine = std::stoi(lineStr);
            pos++;
          } catch (...) {
            throw std::runtime_error("SYNTAX ERROR: INVALID LINE NUMBER");
          }
        } else if (tokens[pos].type == TokenType::NUMBER) {
          startLine = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
      }
    }
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<ChainStmt>(filename, startLine);
  }
  case TokenType::EXEC: {
    pos++; // Skip EXEC
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<ExecStmt>(filename);
  }
  case TokenType::DASH: {
    pos++; // Skip -
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<DashStmt>(filename);
  }
  case TokenType::CAT: {
    pos++; // Skip CAT
    std::string path = ".";
    if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
      path = tokens[pos].value.getString();
      pos++;
    }
    // TODO: Parse optional S# and D# parameters
    return std::make_shared<CatStmt>(path);
  }
  case TokenType::PRODOSREAD: {
    pos++; // Skip READ
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int record = 0, byte = 0;
    // Parse optional ,Rrecord# parameter
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      if (pos < tokens.size()) {
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'R') {
          try {
            std::string recStr = tokens[pos].text.substr(1);
            record = std::stoi(recStr);
            pos++;
          } catch (...) {
            throw std::runtime_error("SYNTAX ERROR: INVALID RECORD NUMBER");
          }
        } else if (tokens[pos].type == TokenType::NUMBER) {
          record = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
      }
      // Parse optional ,Bbyte# parameter
      if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
        pos++; // Skip comma
        if (pos < tokens.size()) {
          if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'B') {
            try {
              std::string byteStr = tokens[pos].text.substr(1);
              byte = std::stoi(byteStr);
              pos++;
            } catch (...) {
              throw std::runtime_error("SYNTAX ERROR: INVALID BYTE NUMBER");
            }
          } else if (tokens[pos].type == TokenType::NUMBER) {
            byte = static_cast<int>(tokens[pos].value.getNumber());
            pos++;
          }
        }
      }
    }
    return std::make_shared<ProdosReadStmt>(filename, record, byte);
  }
  case TokenType::PRODOSWRITE: {
    pos++; // Skip WRITE
    if (pos >= tokens.size() || tokens[pos].type != TokenType::STRING) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED FILENAME");
    }
    std::string filename = tokens[pos].value.getString();
    pos++;
    
    int record = 0;
    // Parse optional ,Rrecord# parameter
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++; // Skip comma
      if (pos < tokens.size()) {
        if (tokens[pos].type == TokenType::IDENTIFIER && tokens[pos].text[0] == 'R') {
          try {
            std::string recStr = tokens[pos].text.substr(1);
            record = std::stoi(recStr);
            pos++;
          } catch (...) {
            throw std::runtime_error("SYNTAX ERROR: INVALID RECORD NUMBER");
          }
        } else if (tokens[pos].type == TokenType::NUMBER) {
          record = static_cast<int>(tokens[pos].value.getNumber());
          pos++;
        }
      }
    }
    return std::make_shared<ProdosWriteStmt>(filename, record);
  }
  case TokenType::IDENTIFIER:
    return parseLetOrAssignment(tokens, pos);
  default:
    pos++;
    return nullptr;
  }
}

/**
 * @brief Parse PRINT statement with expressions and separators
 * 
 * Parses the PRINT statement which outputs values to the screen.
 * Handles multiple expressions separated by commas and semicolons,
 * with different spacing behavior for each separator.
 * 
 * Syntax:
 *   PRINT [expr [separator expr] ...]
 * 
 * Separator behavior:
 *   ; (semicolon): Print next value immediately after current value
 *   , (comma): Tab to next zone (every 14 columns)
 *   none: Print newline after expression
 * 
 * Examples:
 *   PRINT "HELLO"           (prints "HELLO" with newline)
 *   PRINT "A";"B"           (prints "AB" with newline)
 *   PRINT "A","B"           (prints "A" at col 0, "B" at col 14)
 *   PRINT X;                (prints X without newline)
 *   PRINT TAB(10);"TEXT"    (prints "TEXT" at column 10)
 *   PRINT SPC(5);"TEXT"     (prints 5 spaces then "TEXT")
 * 
 * Implementation:
 * - Parses comma-delimited or semicolon-delimited expressions
 * - Trailing separator suppresses newline
 * - Empty PRINT produces a blank line
 * - TAB(n) and SPC(n) are parsed as function calls in expressions
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return PrintStmt with expression list and separator flags
 */
std::shared_ptr<Statement> Parser::parsePrint(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip PRINT

  std::vector<std::shared_ptr<Expression>> exprs;
  std::vector<PrintStmt::Separator> separators;

  while (pos < tokens.size() && tokens[pos].type != TokenType::COLON &&
         tokens[pos].type != TokenType::NEWLINE) {
    auto expr = parseExpression(tokens, pos);
    exprs.push_back(expr);

    PrintStmt::Separator sep = PrintStmt::Separator::None;
    if (pos < tokens.size()) {
      if (tokens[pos].type == TokenType::SEMICOLON) {
        sep = PrintStmt::Separator::Semicolon;
        pos++;
      } else if (tokens[pos].type == TokenType::COMMA) {
        sep = PrintStmt::Separator::Comma;
        pos++;
      }
    }

    separators.push_back(sep);
    if (sep == PrintStmt::Separator::None) {
      break;
    }
  }

  return std::make_shared<PrintStmt>(exprs, separators);
}

/**
 * @brief Parse variable or array assignment statement (LET implementation)
 * 
 * Handles both simple variable assignment and array element assignment.
 * The LET keyword is optional in Applesoft BASIC.
 * 
 * Syntax:
 *   [LET] variable = expression
 *   [LET] array(index1[,index2...]) = expression
 * 
 * Examples:
 *   LET X = 10
 *   Y = X * 2    (LET is optional)
 *   A(5) = 100   (array assignment)
 *   B(I,J) = X+Y (multi-dimensional array)
 * 
 * Implementation:
 * 1. Skip optional LET keyword
 * 2. Parse variable name
 * 3. Check for array indices (parentheses)
 * 4. Require equals sign
 * 5. Parse right-hand side expression
 * 6. Return appropriate statement (LetStmt or ArrayLetStmt)
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return LetStmt for variable or ArrayLetStmt for array assignment
 * @throws std::runtime_error on syntax errors
 */
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

// ============================================================================
// Expression Parsing with Operator Precedence
// ============================================================================
// The following functions implement recursive descent parsing for expressions
// with proper operator precedence. Each level of precedence has its own
// parse function that calls the next higher precedence level.
//
// Precedence hierarchy (lowest to highest):
// 1. OR (logical or)
// 2. AND (logical and)
// 3. NOT (logical not)
// 4. Relational (=, <>, <, >, <=, >=)
// 5. Additive (+, -)
// 6. Multiplicative (*, /, MOD)
// 7. Unary (+, -)
// 8. Power (^)
// 9. Primary (literals, variables, functions, parentheses)
//
// This structure ensures that operations bind correctly:
//   2 + 3 * 4 = 2 + (3 * 4) = 14  (not (2 + 3) * 4 = 20)
//   X = 5 OR Y = 3  evaluates comparisons before OR
// ============================================================================

/**
 * @brief Parse an expression with full operator precedence
 * 
 * Entry point for expression parsing. Dispatches to the lowest precedence
 * level (OR expressions) which then recursively calls higher precedence
 * levels to build the correct AST structure.
 * 
 * @param tokens Token sequence
 * @param pos Current position (updated by parsing)
 * @return Expression AST node
 */
std::shared_ptr<Expression>
Parser::parseExpression(const std::vector<Token> &tokens, size_t &pos) {
  return parseOrExpression(tokens, pos);
}

/**
 * @brief Parse logical OR expressions (lowest precedence)
 * 
 * Handles OR operator which has lowest precedence. Parses left-associative
 * chains of OR operations:
 *   A OR B OR C = (A OR B) OR C
 * 
 * In Applesoft BASIC, OR performs bitwise OR on integer values:
 *   0 OR 0 = 0 (false OR false = false)
 *   0 OR 1 = 1 (false OR true = true)
 *   Any non-zero value is considered true
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing OR chain or single AND expression
 */
std::shared_ptr<Expression>
Parser::parseOrExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parseAndExpression(tokens, pos);

  // Handle left-associative chain: A OR B OR C
  while (pos < tokens.size() && tokens[pos].type == TokenType::OR) {
    pos++;
    auto right = parseAndExpression(tokens, pos);
    left = std::make_shared<BinaryExpr>(left, TokenType::OR, right);
  }

  return left;
}

/**
 * @brief Parse logical AND expressions
 * 
 * Handles AND operator with higher precedence than OR but lower than NOT.
 * Parses left-associative chains:
 *   A AND B AND C = (A AND B) AND C
 * 
 * In Applesoft BASIC, AND performs bitwise AND on integer values:
 *   0 AND 0 = 0 (false AND false = false)
 *   0 AND 1 = 0 (false AND true = false)
 *   1 AND 1 = 1 (true AND true = true)
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing AND chain or single NOT expression
 */
std::shared_ptr<Expression>
Parser::parseAndExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parseNotExpression(tokens, pos);

  // Handle left-associative chain: A AND B AND C
  while (pos < tokens.size() && tokens[pos].type == TokenType::AND) {
    pos++;
    auto right = parseNotExpression(tokens, pos);
    left = std::make_shared<BinaryExpr>(left, TokenType::AND, right);
  }

  return left;
}

/**
 * @brief Parse logical NOT expressions (prefix unary)
 * 
 * Handles NOT operator which has higher precedence than AND/OR.
 * Can be chained: NOT NOT X = X
 * 
 * In Applesoft BASIC, NOT performs bitwise NOT (complement):
 *   NOT 0 = -1 (all bits set)
 *   NOT -1 = 0
 *   NOT X = -(X + 1)
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing NOT operation or relational expression
 */
std::shared_ptr<Expression>
Parser::parseNotExpression(const std::vector<Token> &tokens, size_t &pos) {
  if (pos < tokens.size() && tokens[pos].type == TokenType::NOT) {
    pos++;
    // Recursively handle NOT chains
    auto operand = parseNotExpression(tokens, pos);
    return std::make_shared<NotExpr>(operand);
  }

  return parseRelationalExpression(tokens, pos);
}

/**
 * @brief Parse relational (comparison) expressions
 * 
 * Handles comparison operators: =, <>, <, >, <=, >=
 * Unlike arithmetic operators, comparisons are not chained in BASIC:
 *   A = B = C is parsed as A = (B = C), not (A = B) = C
 * 
 * Comparison results:
 *   True: -1 (all bits set, Applesoft convention)
 *   False: 0
 * 
 * String comparisons use lexicographic ordering.
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing comparison or additive expression
 */
std::shared_ptr<Expression>
Parser::parseRelationalExpression(const std::vector<Token> &tokens,
                                  size_t &pos) {
  auto left = parseAdditiveExpression(tokens, pos);

  // Check for comparison operator (not chained)
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

/**
 * @brief Parse additive expressions (+ and -)
 * 
 * Handles addition and subtraction with left-to-right associativity:
 *   A + B - C + D = ((A + B) - C) + D
 * 
 * String concatenation:
 *   "HELLO" + "WORLD" = "HELLOWORLD"
 * 
 * Type coercion:
 *   Numeric + Numeric = Numeric
 *   String + String = String
 *   String + Numeric or Numeric + String raises TYPE MISMATCH ERROR
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing addition/subtraction or multiplicative expression
 */
std::shared_ptr<Expression>
Parser::parseAdditiveExpression(const std::vector<Token> &tokens, size_t &pos) {
  auto left = parseMultiplicativeExpression(tokens, pos);

  // Handle left-associative chain: A + B - C
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

/**
 * @brief Parse multiplicative expressions (*, /, MOD)
 * 
 * Handles multiplication, division, and modulo with left-to-right associativity:
 *   A * B / C MOD D = ((A * B) / C) MOD D
 * 
 * Operator semantics:
 *   * : Standard multiplication
 *   / : Floating-point division (5 / 2 = 2.5)
 *   MOD : Integer modulo (5 MOD 2 = 1)
 * 
 * Division by zero raises DIVISION BY ZERO ERROR.
 * MOD 0 raises ILLEGAL QUANTITY ERROR.
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing multiplication/division or unary expression
 */
std::shared_ptr<Expression>
Parser::parseMultiplicativeExpression(const std::vector<Token> &tokens,
                                      size_t &pos) {
  auto left = parseUnaryExpression(tokens, pos);

  // Handle left-associative chain: A * B / C
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

/**
 * @brief Parse unary expressions (prefix + and -)
 * 
 * Handles unary plus and minus operators which have higher precedence than
 * binary operators but lower than power (^).
 * 
 * Can be chained:
 *   --X = X (double negation)
 *   -+X = -X
 * 
 * Examples:
 *   -5 = negative five
 *   -X^2 = -(X^2), not (-X)^2
 *   +5 = positive five (usually redundant)
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing unary operation or power expression
 */
std::shared_ptr<Expression>
Parser::parseUnaryExpression(const std::vector<Token> &tokens, size_t &pos) {
  if (pos < tokens.size() && (tokens[pos].type == TokenType::PLUS ||
                              tokens[pos].type == TokenType::MINUS)) {
    TokenType op = tokens[pos].type;
    pos++;
    // Recursively handle unary chains: --X
    auto operand = parseUnaryExpression(tokens, pos);
    return std::make_shared<UnaryExpr>(op, operand);
  }

  return parsePowerExpression(tokens, pos);
}

/**
 * @brief Parse power (exponentiation) expressions
 * 
 * Handles the exponentiation operator (^) which has highest precedence
 * among binary operators.
 * 
 * Right-associative:
 *   2 ^ 3 ^ 2 = 2 ^ (3 ^ 2) = 2 ^ 9 = 512
 *   (not (2 ^ 3) ^ 2 = 8 ^ 2 = 64)
 * 
 * This is the standard mathematical convention for exponentiation.
 * 
 * Examples:
 *   2 ^ 3 = 8
 *   -2 ^ 2 = -(2 ^ 2) = -4 (unary minus has lower precedence)
 *   (-2) ^ 2 = 4
 *   X ^ 0.5 = square root of X
 * 
 * @param tokens Token sequence
 * @param pos Current position
 * @return Expression representing power operation or primary expression
 */
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
      token.type == TokenType::CHR || token.type == TokenType::STR ||
      token.type == TokenType::TAB || token.type == TokenType::SPC ||
      token.type == TokenType::POS || token.type == TokenType::FRE ||
      token.type == TokenType::PDL || token.type == TokenType::PEEK ||
      token.type == TokenType::USR) {
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
  if (token.type == TokenType::LEFT || token.type == TokenType::RIGHT ||
      token.type == TokenType::SCRN) {
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

// (moved class definitions earlier)

/**
 * @brief Parse ON...GOTO or ON...GOSUB computed transfer statement
 * 
 * Implements computed branching based on an integer index value. The index
 * selects which line number to jump to from a list of targets.
 * 
 * Syntax:
 *   ON expression GOTO line1[,line2,...]
 *   ON expression GOSUB line1[,line2,...]
 * 
 * Examples:
 *   ON X GOTO 100,200,300     (if X=1 goto 100, X=2 goto 200, X=3 goto 300)
 *   ON CHOICE GOSUB 1000,2000 (if CHOICE=1 gosub 1000, if CHOICE=2 gosub 2000)
 * 
 * Behavior:
 * - Expression is rounded to integer and 1-indexed
 * - If index < 1 or > list length, continues to next statement (no jump)
 * - GOSUB variant pushes return address on stack
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return OnTransferStmt with index expression and line number list
 * @throws std::runtime_error if GOTO/GOSUB keyword missing or syntax error
 */
std::shared_ptr<Statement> Parser::parseOn(const std::vector<Token> &tokens,
                                           size_t &pos) {
  pos++; // Skip ON
  auto index = parseExpression(tokens, pos);
  if (pos >= tokens.size() || (tokens[pos].type != TokenType::GOTO &&
                               tokens[pos].type != TokenType::GOSUB)) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED GOTO OR GOSUB");
  }
  TokenType kindTok = tokens[pos].type;
  pos++;

  std::vector<int> lines;
  while (pos < tokens.size() && tokens[pos].type != TokenType::NEWLINE &&
         tokens[pos].type != TokenType::COLON) {
    if (tokens[pos].type != TokenType::NUMBER) {
      throw std::runtime_error("SYNTAX ERROR: EXPECTED LINE NUMBER");
    }
    int line = static_cast<int>(tokens[pos].value.getNumber());
    lines.push_back(line);
    pos++;
    if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
      pos++;
      continue;
    }
    break;
  }

  OnTransferStmt::Kind kind = (kindTok == TokenType::GOTO)
                                  ? OnTransferStmt::Goto
                                  : OnTransferStmt::Gosub;
  return std::make_shared<OnTransferStmt>(index, kind, lines);
}

/**
 * @brief Parse INPUT statement for user input
 * 
 * Reads values from user input into variables. Can include an optional
 * prompt string and supports multiple variables in one statement.
 * 
 * Syntax:
 *   INPUT [prompt;] variable[,variable...]
 * 
 * Examples:
 *   INPUT X          (reads number into X)
 *   INPUT A$         (reads string into A$)
 *   INPUT "NAME"; N$ (prompt with semicolon)
 *   INPUT X,Y,Z      (read three values)
 * 
 * Behavior:
 * - Displays "?" prompt if no prompt string provided
 * - Semicolon after prompt suppresses default "?"
 * - Type mismatch causes "?REDO FROM START" message
 * - Multiple values separated by commas in input
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return InputStmt with prompt and variable list
 * @throws std::runtime_error on syntax errors
 */
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

/**
 * @brief Parse IF/THEN/ELSE conditional statement
 * 
 * Parses conditional branching with optional ELSE clause. Supports both
 * statement execution and GOTO-style line number jumps.
 * 
 * Syntax:
 *   IF condition THEN statement[s] [ELSE statement[s]]
 *   IF condition THEN linenum [ELSE linenum]
 * 
 * Examples:
 *   IF X > 10 THEN PRINT "BIG"
 *   IF X > 10 THEN PRINT "BIG" ELSE PRINT "SMALL"
 *   IF X > 10 THEN 100
 *   IF X > 10 THEN 100 ELSE 200
 *   IF A = B THEN PRINT "EQUAL": GOTO 50
 * 
 * Parsing behavior:
 * - Condition must be followed by THEN keyword
 * - If THEN followed by line number, creates implicit GOTO
 * - Otherwise, parses one or more statements until ELSE/colon/newline
 * - ELSE is optional and follows same rules (line number or statements)
 * - Multiple statements in THEN/ELSE must be on same logical line
 * 
 * Implementation notes:
 * - No ENDIF keyword in Applesoft (unlike some BASICs)
 * - Nested IFs are supported: IF A THEN IF B THEN PRINT "BOTH"
 * - Condition evaluates to -1 (true) or 0 (false) in Applesoft
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return IfStmt with condition, THEN statements, and optional ELSE statements
 * @throws std::runtime_error if THEN keyword missing or syntax error
 */
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

/**
 * @brief Parse GOTO statement for unconditional jumps
 * 
 * Parses GOTO which transfers control to a specified line number.
 * This is the fundamental control flow mechanism in BASIC.
 * 
 * Syntax:
 *   GOTO linenum
 * 
 * Examples:
 *   GOTO 100
 *   IF X > 0 THEN GOTO 200
 *   ON N GOTO 100,200,300
 * 
 * Behavior:
 * - Immediately jumps to the specified line number
 * - If line doesn't exist, raises "UNDEF'D STATEMENT ERROR" at runtime
 * - Can jump forward or backward (allows loops)
 * - Does not return (unlike GOSUB)
 * 
 * Implementation:
 * - Simply creates a GotoStmt with the target line number
 * - Runtime interpreter validates line exists and sets program counter
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return GotoStmt with target line number
 * @throws std::runtime_error if line number missing
 */
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

/**
 * @brief Parse GOSUB statement for subroutine calls
 * 
 * Parses GOSUB which calls a subroutine at the specified line number.
 * The subroutine must end with RETURN to return to the calling location.
 * 
 * Syntax:
 *   GOSUB linenum
 * 
 * Examples:
 *   GOSUB 1000
 *   ON N GOSUB 100,200,300
 *   IF ERROR THEN GOSUB 9000
 * 
 * Behavior:
 * - Pushes current line number onto return stack
 * - Jumps to the specified subroutine line
 * - RETURN pops stack and continues at next statement after GOSUB
 * - Supports nested subroutines (limited only by stack depth)
 * - Unmatched RETURN raises "RETURN WITHOUT GOSUB ERROR"
 * 
 * Stack management:
 * - Each GOSUB pushes return address
 * - Each RETURN pops and jumps back
 * - POP statement can discard return address without jumping
 * - CLR and NEW clear the stack
 * 
 * Implementation:
 * - Creates GosubStmt with target line number
 * - Runtime interpreter manages the return stack
 * - Validates target line exists before jumping
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return GosubStmt with target subroutine line number
 * @throws std::runtime_error if line number missing
 */
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

/**
 * @brief Parse FOR loop statement
 * 
 * Parses the start of a FOR/NEXT loop which iterates a control variable
 * from a start value to an end value with an optional step increment.
 * 
 * Syntax:
 *   FOR variable = start TO end [STEP step]
 * 
 * Examples:
 *   FOR I = 1 TO 10
 *   FOR I = 10 TO 1 STEP -1
 *   FOR X = 0 TO 100 STEP 5
 *   FOR I = A TO B STEP C
 * 
 * Loop behavior:
 * - Control variable is set to start value
 * - Loop body executes if variable hasn't passed end value
 * - NEXT increments variable by step (default 1)
 * - Loop continues until variable exceeds end value
 * - Supports negative step for counting down
 * - Nested loops are supported
 * 
 * Edge cases:
 * - If start > end and step > 0, loop body never executes
 * - If start < end and step < 0, loop body never executes
 * - Step of 0 creates infinite loop
 * - Modifying control variable inside loop is allowed but discouraged
 * 
 * Implementation:
 * - Creates ForStmt with variable name, start, end, and optional step
 * - Runtime interpreter manages FOR stack for nested loops
 * - NEXT statement finds matching FOR from stack
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return ForStmt with loop control parameters
 * @throws std::runtime_error on syntax errors (missing =, TO, invalid variable)
 */
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

/**
 * @brief Parse NEXT statement to close FOR loop
 * 
 * Parses NEXT which marks the end of a FOR loop and increments the
 * control variable. Control returns to the matching FOR statement.
 * 
 * Syntax:
 *   NEXT [variable]
 * 
 * Examples:
 *   NEXT
 *   NEXT I
 *   NEXT I,J,K  (close multiple nested loops)
 * 
 * Behavior:
 * - Increments control variable by STEP value
 * - Checks if loop should continue (variable within range)
 * - If continuing, jumps back to statement after FOR
 * - If done, exits loop and continues after NEXT
 * - Variable name is optional but recommended for clarity
 * - If specified, must match innermost active FOR variable
 * 
 * Multiple variables:
 * - NEXT I,J,K closes three nested loops in sequence
 * - Equivalent to: NEXT I: NEXT J: NEXT K
 * - More efficient than separate NEXT statements
 * 
 * Error conditions:
 * - NEXT without FOR raises "NEXT WITHOUT FOR ERROR"
 * - Variable mismatch raises "NEXT WITHOUT FOR ERROR"
 * - Missing NEXT leaves FOR on stack (can cause issues)
 * 
 * Implementation:
 * - Creates NextStmt with optional variable name
 * - Runtime interpreter pops FOR stack and evaluates loop condition
 * - If continuing, sets program counter back to FOR line
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return NextStmt with optional control variable name
 */
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

/**
 * @brief Parse DIM statement for array dimension declaration
 * 
 * Parses DIM which explicitly declares array sizes before use.
 * Without DIM, arrays are automatically dimensioned to size 10 per dimension.
 * 
 * Syntax:
 *   DIM array(size1[,size2,...]) [, array2(...), ...]
 * 
 * Examples:
 *   DIM A(100)              (1D array, indices 0-100)
 *   DIM B(10,10)            (2D array, 11x11 grid)
 *   DIM C(5,5,5)            (3D array)
 *   DIM A(N),B(M,M),C$(50)  (multiple arrays)
 * 
 * Dimension behavior:
 * - Array indices start at 0 and go to declared size (inclusive)
 * - DIM A(10) creates 11 elements: A(0) through A(10)
 * - Dimensions can be expressions: DIM A(N*2)
 * - Arrays can be numeric or string (suffix $)
 * - Once DIM'd, array cannot be redimensioned without CLR
 * 
 * Auto-dimensioning:
 * - Without DIM, first access creates array with size 10 per dimension
 * - A(5) auto-creates A(0) through A(10) if not already DIM'd
 * - Accessing A(15) without DIM raises "BAD SUBSCRIPT ERROR"
 * 
 * Memory management:
 * - Arrays use sparse storage (only used elements stored)
 * - Unused array elements default to 0 (numeric) or "" (string)
 * - CLR deallocates all arrays and variables
 * 
 * Implementation:
 * - Parses comma-separated list of array declarations
 * - Each declaration: name followed by parenthesized dimension expressions
 * - Creates DimStmt with vector of (name, dimensions) entries
 * - Runtime interpreter evaluates dimension expressions and allocates arrays
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return DimStmt with array declarations (name and dimension expressions)
 * @throws std::runtime_error on syntax errors (missing parentheses, invalid format)
 */
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

/**
 * @brief Parse DATA statement for inline data storage
 * 
 * Parses DATA which defines constant values that can be READ by the program.
 * DATA statements are collected before program execution and stored in a
 * sequential buffer accessible via READ/RESTORE.
 * 
 * Syntax:
 *   DATA value1[, value2, value3, ...]
 * 
 * Examples:
 *   DATA 10, 20, 30
 *   DATA "HELLO", "WORLD"
 *   DATA 3.14159, "PI", 2.71828, "E"
 *   DATA -100, 0, 100
 * 
 * Data storage:
 * - All DATA statements are collected into a single sequential list
 * - Order is determined by program line numbers, not execution order
 * - Values can be numbers or strings
 * - Commas separate values within and across DATA statements
 * - DATA is processed before RUN, so control flow doesn't affect availability
 * 
 * Usage with READ:
 * - READ retrieves values sequentially from the DATA list
 * - READ advances the data pointer after each value
 * - RESTORE resets pointer to beginning or a specific line's DATA
 * - Reading past end raises "OUT OF DATA ERROR"
 * 
 * Common patterns:
 *   100 DATA 5
 *   110 READ N
 *   120 FOR I = 1 TO N
 *   130 READ A(I)
 *   140 NEXT
 *   200 DATA 10,20,30,40,50
 * 
 * Implementation:
 * - Parses comma-separated list of literal values
 * - Creates DataStmt with vector of values
 * - Interpreter's runFrom() collects all DATA before execution starts
 * - Data pointer tracks current READ position
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return DataStmt with vector of constant values
 * @throws std::runtime_error if non-literal value found
 */
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

/**
 * @brief Parse READ statement to retrieve DATA values
 * 
 * Parses READ which assigns values from DATA statements to variables.
 * Values are read sequentially from the DATA list maintained by the interpreter.
 * 
 * Syntax:
 *   READ variable1[, variable2, variable3, ...]
 * 
 * Examples:
 *   READ A
 *   READ A, B, C
 *   READ A(I), B$(J)
 *   READ X, Y$, Z(I,J)
 * 
 * Sequential reading:
 * - Each READ advances the data pointer through the DATA list
 * - Multiple READ statements continue from where previous READ left off
 * - Data pointer persists across program runs until RESTORE or NEW
 * - Reading past end raises "OUT OF DATA ERROR"
 * 
 * Type matching:
 * - Numeric variables read numeric DATA values
 * - String variables read any DATA value (converted to string if needed)
 * - Type mismatch raises "TYPE MISMATCH ERROR"
 * 
 * Array element reading:
 * - READ can assign to array elements: READ A(I)
 * - Subscript expressions are evaluated at READ time
 * - Supports multi-dimensional arrays: READ B(I,J)
 * 
 * RESTORE interaction:
 * - RESTORE resets data pointer to beginning
 * - RESTORE linenum positions pointer at specific line's DATA
 * - Allows re-reading DATA or jumping to different data sections
 * 
 * Common patterns:
 *   10 READ N
 *   20 FOR I = 1 TO N
 *   30 READ A(I)
 *   40 NEXT I
 *   100 DATA 5, 10, 20, 30, 40, 50
 * 
 * Implementation:
 * - Parses comma-separated list of variables or array subscripts
 * - Creates ReadStmt with vector of targets (variable or array element)
 * - Runtime interpreter pulls values from dataValues_ array
 * - Handles type coercion and bounds checking
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return ReadStmt with list of target variables/arrays
 * @throws std::runtime_error on syntax errors
 */
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

/**
 * @brief Parse DEF FN statement for user-defined functions
 * 
 * Parses DEF FN which defines a single-line user function that can be
 * called like a built-in function. Function names must start with "FN".
 * 
 * Syntax:
 *   DEF FNname(parameter) = expression
 * 
 * Examples:
 *   DEF FNA(X) = X * X
 *   DEF FNB(Y) = SQR(Y * Y + 1)
 *   DEF FNC(Z) = INT(Z + 0.5)
 *   DEF FNMAX(A,B) = (A + B + ABS(A - B)) / 2  (Applesoft supports only 1 param)
 * 
 * Function naming:
 * - Must start with FN followed by 1-2 letters
 * - Name significance: FN + first 2 chars (FNxy, FNab, etc.)
 * - Case insensitive: FNA, FNa, and Fna are the same
 * - FN can be written as separate keyword or part of name
 * 
 * Function behavior:
 * - Single parameter only (Applesoft limitation)
 * - Expression can reference the parameter and any global variables
 * - Function call: FNA(10) evaluates DEF expression with parameter = 10
 * - Recursive calls are supported but uncommon
 * - Functions are stored in the Variables object
 * 
 * Scoping:
 * - Parameter is local to the function expression
 * - All other variables are global
 * - Functions can reference and modify global variables
 * - Functions defined at any point can be called from any line
 * 
 * Example usage:
 *   100 DEF FNS(X) = X * X
 *   110 FOR I = 1 TO 10
 *   120 PRINT FNS(I)
 *   130 NEXT I
 * 
 * Implementation:
 * - Parses FN keyword + name, parenthesized parameter, equals, expression
 * - Creates DefStmt with function name, parameter, and expression AST
 * - Runtime interpreter stores function definition in Variables
 * - Function calls are parsed as FunctionCallExpr and evaluated by substituting parameter
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return DefStmt with function name, parameter, and expression
 * @throws std::runtime_error on syntax errors (missing FN prefix, invalid format)
 */
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

/**
 * @brief Parse ONERR GOTO statement for error handling
 * 
 * Parses ONERR GOTO which sets up an error handler that catches runtime errors.
 * When an error occurs, control transfers to the specified line number instead
 * of stopping the program.
 * 
 * Syntax:
 *   ONERR GOTO linenum
 * 
 * Examples:
 *   ONERR GOTO 9000
 *   IF DEBUG THEN ONERR GOTO 0  (disable error handling)
 * 
 * Error handling behavior:
 * - When runtime error occurs, jumps to specified line
 * - Error information available via PEEK(222) for error code
 * - ERR variable contains error code (if supported)
 * - ERL variable contains line where error occurred (if supported)
 * - Program continues from error handler unless explicit END/STOP
 * 
 * Special cases:
 * - ONERR GOTO 0 disables error handler (returns to normal error behavior)
 * - Only one error handler can be active at a time
 * - Setting new ONERR replaces previous handler
 * - CLR and NEW disable error handler
 * 
 * RESUME statement:
 * - RESUME continues at the line that caused the error
 * - RESUME NEXT continues at the line after the error
 * - RESUME linenum continues at specified line
 * - Using RESUME outside error handler raises error
 * 
 * Common error handling pattern:
 *   10 ONERR GOTO 1000
 *   20 INPUT "FILE"; F$
 *   30 OPEN F$
 *   ...
 *   1000 REM Error handler
 *   1010 PRINT "ERROR "; PEEK(222)
 *   1020 RESUME NEXT
 * 
 * ProDOS error codes (PEEK(222)):
 *   0: No error
 *   4: File not found
 *   5: Volume not found
 *   46: Disk full
 *   (and many others - see Applesoft documentation)
 * 
 * Implementation:
 * - Parses ONERR keyword, GOTO keyword, and line number
 * - Creates OnErrStmt with target line number
 * - Runtime interpreter stores error handler line in state
 * - When error occurs, checks if handler is set and jumps there
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return OnErrStmt with error handler line number
 * @throws std::runtime_error if GOTO keyword or line number missing
 */
std::shared_ptr<Statement> Parser::parseOnErr(const std::vector<Token> &tokens,
                                              size_t &pos) {
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

/**
 * @brief Parse RANDOMIZE statement to seed random number generator
 * 
 * Seeds the random number generator for RND() function. If no seed is
 * provided, implementation uses current time or default seed.
 * 
 * Syntax:
 *   RANDOMIZE [seed]
 * 
 * Examples:
 *   RANDOMIZE       (use default/time-based seed)
 *   RANDOMIZE 42    (explicit seed for reproducible results)
 *   RANDOMIZE X*10  (seed can be an expression)
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return RandomizeStmt with optional seed expression
 */
std::shared_ptr<Statement>
Parser::parseRandomize(const std::vector<Token> &tokens, size_t &pos) {
  pos++; // Skip RANDOMIZE
  std::shared_ptr<Expression> seed;
  if (pos < tokens.size() && tokens[pos].type != TokenType::COLON &&
      tokens[pos].type != TokenType::NEWLINE) {
    seed = parseExpression(tokens, pos);
  }
  return std::make_shared<RandomizeStmt>(seed);
}

/**
 * @brief Parse SPEED statement for execution delay control
 * 
 * Sets a delay in milliseconds between each statement execution, useful
 * for debugging or creating visual effects in programs.
 * 
 * Syntax:
 *   SPEED [=] delay
 * 
 * Examples:
 *   SPEED = 100   (100ms delay between statements)
 *   SPEED 0       (no delay - full speed)
 *   SPEED 255     (maximum delay - 255ms)
 * 
 * The delay value should be 0-255. Values outside this range will be
 * clamped during execution.
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return SpeedStmt with delay expression
 */
std::shared_ptr<Statement> Parser::parseSpeed(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip SPEED
  if (pos < tokens.size() && tokens[pos].type == TokenType::EQUAL) {
    pos++; // Optional '='
  }
  auto delay = parseExpression(tokens, pos);
  return std::make_shared<SpeedStmt>(delay);
}

/**
 * @brief Parse device I/O redirection (PR# and IN# statements)
 * 
 * Handles ProDOS-style device redirection for output (PR#) and input (IN#).
 * These commands were used in Apple II to redirect I/O to different slots
 * (printers, modems, disk drives, etc.).
 * 
 * Syntax:
 *   PR[#] slot    (redirect output to slot)
 *   IN[#] slot    (redirect input from slot)
 * 
 * Examples:
 *   PR#1          (redirect output to printer in slot 1)
 *   PR#0          (restore output to screen)
 *   IN#2          (redirect input from slot 2)
 *   IN#0          (restore input from keyboard)
 * 
 * Note: This implementation provides stub support for compatibility;
 * actual device redirection depends on platform capabilities.
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @param isOutput true for PR# (output), false for IN# (input)
 * @return DeviceRedirectStmt with slot expression
 */
std::shared_ptr<Statement>
Parser::parseDeviceRedirect(const std::vector<Token> &tokens, size_t &pos,
                            bool isOutput) {
  pos++; // Skip PR or IN
  if (pos < tokens.size() && tokens[pos].type == TokenType::HASH) {
    pos++; // Optional '#'
  }
  auto slot = parseExpression(tokens, pos);
  if (isOutput) {
    return std::make_shared<PrStmt>(slot);
  }
  return std::make_shared<InStmt>(slot);
}

/**
 * @brief Parse WHILE statement for conditional loops
 * 
 * Parses WHILE which starts a loop that executes while a condition is true.
 * The loop continues until the condition becomes false or an explicit exit.
 * Must be closed with WEND statement.
 * 
 * Syntax:
 *   WHILE condition
 *     [statements]
 *   WEND
 * 
 * Examples:
 *   WHILE X < 100
 *     X = X + 1
 *   WEND
 * 
 *   WHILE A$ <> "QUIT"
 *     INPUT A$
 *   WEND
 * 
 * Loop behavior:
 * - Condition is evaluated at loop start (before statements)
 * - If condition is false initially, loop body never executes
 * - Condition is re-evaluated each time WEND is reached
 * - Loop continues while condition is non-zero (true)
 * - Loop exits when condition becomes zero (false)
 * 
 * Condition evaluation:
 * - Numeric: 0 = false, any non-zero = true (typically -1)
 * - String: empty "" = false, non-empty = true
 * - Expressions: result compared to zero
 * 
 * Nesting:
 * - WHILE loops can be nested
 * - Each WHILE must have matching WEND
 * - Can nest with FOR/NEXT loops
 * - Improper nesting causes runtime errors
 * 
 * Exiting loops:
 * - Normal exit: condition becomes false
 * - GOTO can jump out of loop
 * - No explicit BREAK statement in Applesoft
 * - Missing WEND causes program to continue past loop
 * 
 * Comparison with FOR:
 * - WHILE is condition-based, FOR is count-based
 * - WHILE more flexible for unknown iteration counts
 * - FOR better for fixed ranges with counter
 * 
 * Implementation:
 * - Parses WHILE keyword and condition expression
 * - Creates WhileStmt with condition
 * - Return line number set at runtime by interpreter
 * - WEND statement pops while stack and jumps back to WHILE line
 * - Condition re-evaluated each iteration
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return WhileStmt with condition expression
 */
std::shared_ptr<Statement> Parser::parseWhile(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip WHILE
  auto condition = parseExpression(tokens, pos);
  return std::make_shared<WhileStmt>(condition,
                                     -1); // returnLine set at runtime
}

/**
 * @brief Parse WAIT statement for memory polling
 * 
 * Waits for a memory location to match a specific bit pattern, with
 * optional timeout. This was used in Applesoft to wait for hardware
 * status changes.
 * 
 * Syntax:
 *   WAIT address, mask[, timeout]
 * 
 * Examples:
 *   WAIT 49152,128        (wait for bit 7 at address 49152)
 *   WAIT -16384,128       (same - negative addresses supported)
 *   WAIT 49152,128,1000   (wait up to 1000ms)
 * 
 * Behavior:
 * - Loops until (PEEK(address) AND mask) != 0
 * - Optional third parameter: timeout in milliseconds
 * - Without timeout, waits indefinitely (use Ctrl+C to break)
 * - With timeout, exits silently when time expires
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return WaitStmt with address, mask, and optional timeout
 * @throws std::runtime_error on syntax errors
 */
std::shared_ptr<Statement> Parser::parseWait(const std::vector<Token> &tokens,
                                             size_t &pos) {
  pos++; // Skip WAIT
  auto addr = parseExpression(tokens, pos);
  if (pos >= tokens.size() || tokens[pos].type != TokenType::COMMA) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED COMMA");
  }
  pos++;
  auto mask = parseExpression(tokens, pos);
  // Optional third argument: timeout in milliseconds
  std::shared_ptr<Expression> timeout;
  if (pos < tokens.size() && tokens[pos].type == TokenType::COMMA) {
    pos++;
    timeout = parseExpression(tokens, pos);
  }
  return std::make_shared<WaitStmt>(addr, mask, timeout);
}

/**
 * @brief Parse HIMEM statement to set upper memory limit
 * 
 * Sets the highest memory address that BASIC can use for variables and
 * arrays. Memory above HIMEM is protected from BASIC programs.
 * 
 * Syntax:
 *   HIMEM = address
 * 
 * Examples:
 *   HIMEM = 32767    (set upper limit to 32767)
 *   HIMEM = 16384    (protect upper half of memory)
 * 
 * Behavior:
 * - HIMEM must be > LOMEM
 * - PEEK/POKE operations outside LOMEM-HIMEM range will error
 * - Used to reserve memory for machine language routines or data
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return HimemStmt with address expression
 * @throws std::runtime_error if equals sign missing
 */
std::shared_ptr<Statement> Parser::parseHimem(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip HIMEM
  if (pos >= tokens.size() || tokens[pos].type != TokenType::EQUAL) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED =");
  }
  pos++;
  auto addr = parseExpression(tokens, pos);
  return std::make_shared<HimemStmt>(addr);
}

/**
 * @brief Parse LOMEM statement to set lower memory limit
 * 
 * Sets the lowest memory address that BASIC can use for variables and
 * arrays. Memory below LOMEM is protected from BASIC programs.
 * 
 * Syntax:
 *   LOMEM = address
 * 
 * Examples:
 *   LOMEM = 2048     (set lower limit to 2048)
 *   LOMEM = 8192     (protect lower 8K for machine code)
 * 
 * Behavior:
 * - LOMEM must be < HIMEM
 * - PEEK/POKE operations outside LOMEM-HIMEM range will error
 * - Used to reserve memory for machine language routines
 * - Often set before loading assembly language programs with BLOAD
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return LomemStmt with address expression
 * @throws std::runtime_error if equals sign missing
 */
std::shared_ptr<Statement> Parser::parseLomem(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip LOMEM
  if (pos >= tokens.size() || tokens[pos].type != TokenType::EQUAL) {
    throw std::runtime_error("SYNTAX ERROR: EXPECTED =");
  }
  pos++;
  auto addr = parseExpression(tokens, pos);
  return std::make_shared<LomemStmt>(addr);
}

/**
 * @brief Parse SHLOAD statement to load shape table
 * 
 * Loads a shape table from tape or disk for use with DRAW and XDRAW
 * commands. Shape tables contain vector graphics data.
 * 
 * Syntax:
 *   SHLOAD [filename]
 * 
 * Examples:
 *   SHLOAD           (load from current tape position)
 *   SHLOAD "SHAPES"  (load from file named "SHAPES")
 * 
 * Behavior:
 * - Without filename: reads from current tape position (sequential)
 * - With filename: loads from disk file
 * - Shape table pointer stored in memory locations 232-233 (0xE8-0xE9)
 * - After loading, shapes can be drawn with DRAW/XDRAW
 * 
 * @param tokens Token sequence to parse
 * @param pos Current position (updated after parsing)
 * @return ShloadStmt with optional filename
 */
std::shared_ptr<Statement> Parser::parseShload(const std::vector<Token> &tokens,
                                               size_t &pos) {
  pos++; // Skip SHLOAD
  
  // Check if there's a filename parameter
  if (pos < tokens.size() && tokens[pos].type == TokenType::STRING) {
    std::string filename = tokens[pos].value.getString();
    pos++;
    return std::make_shared<ShloadStmt>(filename);
  }
  
  // No filename, use DATA statements
  return std::make_shared<ShloadStmt>();
}

/**
 * @brief Check if current token matches expected type (parser helper)
 * 
 * Tests whether the token at the specified position matches the given type
 * without consuming it. This is a lookahead helper used throughout the parser
 * for decision-making without advancing the parse position.
 * 
 * Usage in parsing:
 * - Check for optional tokens (e.g., THEN after IF)
 * - Lookahead for operator precedence decisions
 * - Validate token sequences
 * 
 * @param tokens Token vector being parsed
 * @param pos Current position in token vector
 * @param type Expected token type to match
 * @return true if token at pos matches type, false otherwise
 */
bool Parser::match(const std::vector<Token> &tokens, size_t pos,
                   TokenType type) const {
  return pos < tokens.size() && tokens[pos].type == type;
}

/**
 * @brief Check if at end of token stream (parser helper)
 * 
 * Determines whether the parser has consumed all tokens. Used to detect
 * end of statement or expression parsing.
 * 
 * @param tokens Token vector being parsed
 * @param pos Current position in token vector
 * @return true if position is at or past end of token vector
 */
bool Parser::isAtEnd(const std::vector<Token> &tokens, size_t pos) const {
  return pos >= tokens.size();
}
