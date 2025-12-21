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

// Statement implementations for WHILE/WEND/POP
void WhileStmt::execute(Interpreter *interp) {
  interp->pushWhileLoop(condition_, interp->getCurrentLine());
}

void WendStmt::execute(Interpreter *interp) { interp->nextWhileLoop(); }

void PopStmt::execute(Interpreter *interp) { interp->popGosub(); }

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
  auto left = parseNotExpression(tokens, pos);

  while (pos < tokens.size() && tokens[pos].type == TokenType::AND) {
    pos++;
    auto right = parseNotExpression(tokens, pos);
    left = std::make_shared<BinaryExpr>(left, TokenType::AND, right);
  }

  return left;
}

std::shared_ptr<Expression>
Parser::parseNotExpression(const std::vector<Token> &tokens, size_t &pos) {
  if (pos < tokens.size() && tokens[pos].type == TokenType::NOT) {
    pos++;
    auto operand = parseNotExpression(tokens, pos);
    return std::make_shared<NotExpr>(operand);
  }

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

std::shared_ptr<Statement> Parser::parseSpeed(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip SPEED
  if (pos < tokens.size() && tokens[pos].type == TokenType::EQUAL) {
    pos++; // Optional '='
  }
  auto delay = parseExpression(tokens, pos);
  return std::make_shared<SpeedStmt>(delay);
}

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

std::shared_ptr<Statement> Parser::parseWhile(const std::vector<Token> &tokens,
                                              size_t &pos) {
  pos++; // Skip WHILE
  auto condition = parseExpression(tokens, pos);
  return std::make_shared<WhileStmt>(condition,
                                     -1); // returnLine set at runtime
}

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

bool Parser::match(const std::vector<Token> &tokens, size_t pos,
                   TokenType type) const {
  return pos < tokens.size() && tokens[pos].type == type;
}

bool Parser::isAtEnd(const std::vector<Token> &tokens, size_t pos) const {
  return pos >= tokens.size();
}
