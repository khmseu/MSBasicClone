#pragma once

#include "functions.h"
#include "parser.h"
#include "types.h"
#include "variables.h"
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

class Interpreter {
public:
  Interpreter();

  // Program management
  void loadProgram(const std::string &filename);
  void saveProgram(const std::string &filename);
  void newProgram();
  void listProgram(int startLine = -1, int endLine = -1);
  void addLine(LineNumber lineNum, const std::string &text);
  void deleteLine(LineNumber lineNum);

  // Execution
  void run();
  void runFrom(LineNumber lineNum);
  void executeImmediate(const std::string &line);

  // Interactive mode
  void interactive();

  // Variable access (for expressions)
  Variables &getVariables() { return variables_; }

  // Get current line number
  LineNumber getCurrentLine() const { return currentLine_; }

  // Execution control
  void gotoLine(LineNumber lineNum);
  void gosub(LineNumber lineNum);
  void returnFromGosub();
  void endProgram();
  void stop();
  void cont();

  // Output helpers
  void htab(int col1); // horizontal tab to column (1-based)
  void vtab(int row1); // vertical tab to row (1-based)
  void setInverse(bool on);
  void setFlash(bool on);
  void setNormal();
  void printText(const std::string &text);
  void printNewline();
  void printToNextZone();
  void resetOutputPosition();

  // Mode helpers
  bool isImmediateMode() const { return immediate_; }

  // Data statements
  void addDataValue(const Value &value);
  Value readData();
  void restoreData(int line = -1);

  // FOR loops
  void pushForLoop(const std::string &varName, double endValue,
                   double stepValue, LineNumber returnLine);
  bool isInForLoop(const std::string &varName);
  void nextForLoop(const std::string &varName);

  // Error handling
  void setErrorHandler(LineNumber lineNum);
  void handleError(const std::string &message);
  void resume();

  // Debugging
  void setTrace(bool on) { tracing_ = on; }
  bool isTracing() const { return tracing_; }

  // Random number seeding
  void randomize(double seed);

  // SPEED delay control (milliseconds, clamped 0-255)
  void setSpeedDelay(int delayMs);
  int getSpeedDelay() const { return speedDelayMs_; }

  // Device redirection stubs
  void setOutputDevice(int slot);
  void setInputDevice(int slot);
  int getOutputDevice() const { return outputDevice_; }
  int getInputDevice() const { return inputDevice_; }

  // WHILE loops
  void pushWhileLoop(std::shared_ptr<Expression> condition,
                     LineNumber returnLine);
  void nextWhileLoop();

  // Memory management
  void setHimem(int val) {
    himem_ = val;
    // Keep memory helpers in sync
    setMemoryBounds(lomem_, himem_);
  }
  void setLomem(int val) {
    lomem_ = val;
    setMemoryBounds(lomem_, himem_);
  }
  int getHimem() const { return himem_; }
  int getLomem() const { return lomem_; }

  // GOSUB stack manipulation (for POP)
  void popGosub();

  // File system operations
  void catalog();

  // Array persistence
  void storeArray(const std::string &arrayName);
  void recallArray(const std::string &arrayName);

  // State reset
  void clearState();

private:
  Variables variables_;
  std::map<LineNumber, ProgramLine> program_;

  // Execution state
  LineNumber currentLine_;
  bool running_;
  bool immediate_;
  bool jumped_; // Flag to prevent auto-increment after GOTO/GOSUB
  std::map<LineNumber, ProgramLine>::iterator programCounter_;
  bool paused_ = false;
  LineNumber continueAfterLine_ = -1;

  // GOSUB stack
  std::stack<LineNumber> gosubStack_;

  // FOR loop tracking
  struct ForLoopInfo {
    std::string varName;
    double endValue;
    double stepValue;
    LineNumber returnLine;
  };
  std::vector<ForLoopInfo> forStack_;

  // DATA/READ support
  std::vector<Value> dataValues_;
  size_t dataPointer_;
  std::vector<std::pair<int, size_t>> dataOffsets_;

  // Error handling
  LineNumber errorHandlerLine_;
  std::string lastError_;
  LineNumber errorLine_;

  // Debugging
  bool tracing_ = false;

  // Output state
  int outputColumn_ = 0;
  int outputRow_ = 0;
  bool inverse_ = false;
  bool flash_ = false;
  bool vtEnabled_ = true;

  // SPEED delay (ms), PR#/IN# slot tracking
  int speedDelayMs_ = 0;
  int outputDevice_ = 0;
  int inputDevice_ = 0;

  // Memory boundaries (HIMEM/LOMEM)
  int himem_ = 49152; // Default to $C000
  int lomem_ = 2048;  // Default to $800

  // WHILE loop tracking
  struct WhileLoopInfo {
    std::shared_ptr<Expression> condition;
    LineNumber returnLine;
  };
  std::vector<WhileLoopInfo> whileStack_;

  // Helper methods
  void parseLine(const std::string &line, LineNumber &lineNum,
                 std::string &code);
  bool isLineNumber(const std::string &text) const;
  void updateTextAttributes();
  void applySpeedDelay();
};
