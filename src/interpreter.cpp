#include "interpreter.h"
#include "filesystem.h"
#include "interactive.h"
#include "parser.h"
#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

Interpreter::Interpreter()
    : currentLine_(0), running_(false), immediate_(false), jumped_(false),
      dataPointer_(0), errorHandlerLine_(-1), errorLine_(-1) {}

void Interpreter::parseLine(const std::string &line, LineNumber &lineNum,
                            std::string &code) {
  // Check if line starts with a number
  size_t pos = 0;
  while (pos < line.length() && std::isspace(line[pos]))
    pos++;

  if (pos < line.length() && std::isdigit(line[pos])) {
    // Extract line number
    size_t numEnd = pos;
    while (numEnd < line.length() && std::isdigit(line[numEnd]))
      numEnd++;

    lineNum = std::stoi(line.substr(pos, numEnd - pos));

    // Rest is code
    while (numEnd < line.length() && std::isspace(line[numEnd]))
      numEnd++;
    code = line.substr(numEnd);
  } else {
    lineNum = -1;
    code = line;
  }
}

bool Interpreter::isLineNumber(const std::string &text) const {
  if (text.empty())
    return false;
  for (char c : text) {
    if (!std::isdigit(c))
      return false;
  }
  return true;
}

void Interpreter::addLine(LineNumber lineNum, const std::string &text) {
  if (text.empty()) {
    // Empty line deletes the line
    program_.erase(lineNum);
  } else {
    ProgramLine pline;
    pline.lineNumber = lineNum;
    pline.text = text;

    Tokenizer tokenizer;
    pline.tokens = tokenizer.tokenize(text);

    Parser parser;
    pline.statements = parser.parse(pline.tokens);

    program_[lineNum] = pline;
  }
}

void Interpreter::deleteLine(LineNumber lineNum) { program_.erase(lineNum); }

void Interpreter::newProgram() {
  program_.clear();
  variables_.clear();
  dataValues_.clear();
  dataPointer_ = 0;
}

void Interpreter::listProgram(int startLine, int endLine) {
  for (const auto &pair : program_) {
    if ((startLine < 0 || pair.first >= startLine) &&
        (endLine < 0 || pair.first <= endLine)) {
      std::cout << pair.first << " " << pair.second.text << "\n";
    }
  }
}

void Interpreter::run() { runFrom(-1); }

void Interpreter::runFrom(LineNumber lineNum) {
  running_ = true;
  immediate_ = false;

  // Prepare DATA cache before execution so READ works regardless of control
  // flow.
  dataPointer_ = 0;
  dataValues_.clear();
  for (const auto &pair : program_) {
    for (const auto &stmt : pair.second.statements) {
      stmt->collectData(dataValues_);
    }
  }

  if (lineNum < 0 && !program_.empty()) {
    programCounter_ = program_.begin();
  } else {
    programCounter_ = program_.find(lineNum);
    if (programCounter_ == program_.end()) {
      std::cout << "?UNDEF'D STATEMENT ERROR\n";
      return;
    }
  }

  try {
    while (running_ && programCounter_ != program_.end()) {
      currentLine_ = programCounter_->first;
      jumped_ = false;

      for (auto &stmt : programCounter_->second.statements) {
        stmt->execute(this);
        if (!running_ || jumped_)
          break;
      }

      if (!jumped_) {
        ++programCounter_;
      }
    }
  } catch (const std::exception &e) {
    if (errorHandlerLine_ >= 0) {
      errorLine_ = currentLine_;
      lastError_ = e.what();
      gotoLine(errorHandlerLine_);
    } else {
      std::cout << "?" << e.what() << " IN LINE " << currentLine_ << "\n";
      running_ = false;
    }
  }

  running_ = false;
}

void Interpreter::executeImmediate(const std::string &line) {
  LineNumber lineNum;
  std::string code;

  parseLine(line, lineNum, code);

  if (lineNum >= 0) {
    // Line with number - add to program
    addLine(lineNum, code);
  } else {
    // Immediate command
    std::string upperCode = code;
    std::transform(upperCode.begin(), upperCode.end(), upperCode.begin(),
                   ::toupper);

    if (upperCode == "RUN") {
      run();
    } else if (upperCode == "LIST") {
      listProgram();
    } else if (upperCode == "NEW") {
      newProgram();
    } else if (upperCode.substr(0, 4) == "LOAD") {
      std::string filename = code.substr(4);
      // Trim spaces
      filename.erase(0, filename.find_first_not_of(" \t"));
      filename.erase(filename.find_last_not_of(" \t") + 1);
      loadProgram(filename);
    } else if (upperCode.substr(0, 4) == "SAVE") {
      std::string filename = code.substr(4);
      filename.erase(0, filename.find_first_not_of(" \t"));
      filename.erase(filename.find_last_not_of(" \t") + 1);
      saveProgram(filename);
    } else if (upperCode == "CATALOG" || upperCode == "CAT") {
      catalog();
    } else {
      // Execute as immediate statement
      immediate_ = true;

      Tokenizer tokenizer;
      std::vector<Token> tokens = tokenizer.tokenize(code);

      Parser parser;
      std::vector<std::shared_ptr<Statement>> statements = parser.parse(tokens);

      for (auto &stmt : statements) {
        stmt->execute(this);
      }

      immediate_ = false;
    }
  }
}

void Interpreter::gotoLine(LineNumber lineNum) {
  auto it = program_.find(lineNum);
  if (it == program_.end()) {
    throw std::runtime_error("UNDEF'D STATEMENT ERROR");
  }
  programCounter_ = it;
  jumped_ = true;
}

void Interpreter::gosub(LineNumber lineNum) {
  gosubStack_.push(currentLine_);
  auto it = program_.find(lineNum);
  if (it == program_.end()) {
    throw std::runtime_error("UNDEF'D STATEMENT ERROR");
  }
  programCounter_ = it;
  jumped_ = true;
}

void Interpreter::returnFromGosub() {
  if (gosubStack_.empty()) {
    throw std::runtime_error("RETURN WITHOUT GOSUB ERROR");
  }
  LineNumber returnLine = gosubStack_.top();
  gosubStack_.pop();

  // Continue after the GOSUB line
  auto it = program_.find(returnLine);
  if (it != program_.end()) {
    ++it;
    programCounter_ = it;
    jumped_ = true;
  }
}

void Interpreter::endProgram() { running_ = false; }

void Interpreter::loadProgram(const std::string &filename) {
  try {
    std::string content = readTextFile(filename);
    newProgram();

    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
      if (!line.empty()) {
        LineNumber lineNum;
        std::string code;
        parseLine(line, lineNum, code);
        if (lineNum >= 0) {
          addLine(lineNum, code);
        }
      }
    }
  } catch (const std::exception &e) {
    std::cout << "?" << e.what() << "\n";
  }
}

void Interpreter::saveProgram(const std::string &filename) {
  try {
    std::ostringstream oss;
    for (const auto &pair : program_) {
      oss << pair.first << " " << pair.second.text << "\n";
    }
    writeTextFile(filename, oss.str());
  } catch (const std::exception &e) {
    std::cout << "?" << e.what() << "\n";
  }
}

void Interpreter::catalog() {
  auto files = listFiles(".");

  std::cout << "\nCATALOG\n\n";
  for (const auto &file : files) {
    if (!file.isDirectory && file.name.length() > 0 && file.name[0] != '.') {
      std::cout << " " << file.name << "\n";
    }
  }
  std::cout << "\n";
}

void Interpreter::addDataValue(const Value &value) {
  dataValues_.push_back(value);
}

Value Interpreter::readData() {
  if (dataPointer_ >= dataValues_.size()) {
    throw std::runtime_error("OUT OF DATA ERROR");
  }
  return dataValues_[dataPointer_++];
}

void Interpreter::restoreData() { dataPointer_ = 0; }

void Interpreter::pushForLoop(const std::string &varName, double endValue,
                              double stepValue, LineNumber returnLine) {
  ForLoopInfo info;
  info.varName = varName;
  info.endValue = endValue;
  info.stepValue = stepValue;
  info.returnLine = returnLine;
  forStack_.push_back(info);
}

bool Interpreter::isInForLoop(const std::string &varName) {
  for (const auto &loop : forStack_) {
    if (loop.varName == varName)
      return true;
  }
  return false;
}

void Interpreter::nextForLoop(const std::string &varName) {
  // Find matching FOR loop
  for (auto it = forStack_.rbegin(); it != forStack_.rend(); ++it) {
    if (it->varName == varName || varName.empty()) {
      // Increment variable
      Value current = variables_.getVariable(it->varName);
      double newVal = current.getNumber() + it->stepValue;
      variables_.setVariable(it->varName, Value(newVal));

      // Check if loop should continue
      bool shouldContinue;
      if (it->stepValue >= 0) {
        shouldContinue = newVal <= it->endValue;
      } else {
        shouldContinue = newVal >= it->endValue;
      }

      if (shouldContinue) {
        // Jump back to line after FOR
        auto lineIt = program_.find(it->returnLine);
        if (lineIt != program_.end()) {
          ++lineIt; // Move to next line
          programCounter_ = lineIt;
          jumped_ = true;
        }
      } else {
        // Loop complete, remove from stack
        forStack_.erase(std::next(it).base());
      }
      return;
    }
  }

  throw std::runtime_error("NEXT WITHOUT FOR ERROR");
}

void Interpreter::setErrorHandler(LineNumber lineNum) {
  errorHandlerLine_ = lineNum;
}

void Interpreter::handleError(const std::string &message) {
  throw std::runtime_error(message);
}

void Interpreter::resume() {
  if (errorLine_ < 0) {
    throw std::runtime_error("RESUME WITHOUT ERROR");
  }
  gotoLine(errorLine_);
  errorLine_ = -1;
}

void Interpreter::interactive() {
  InteractiveMode interactive;
  interactive.run();
}
