#include "interpreter.h"
#include "filesystem.h"
#include "float40.h"
#include "interactive.h"
#include "parser.h"
#include "tokenizer.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#ifdef _WIN32
#include <windows.h>
#endif

Interpreter::Interpreter(const GraphicsConfig& config)
    : currentLine_(0), running_(false), immediate_(false), jumped_(false),
      dataPointer_(0), errorHandlerLine_(-1), errorLine_(-1), graphicsConfig_(config) {
#ifdef _WIN32
  auto enableVirtualTerminal = []() -> bool {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == nullptr || hOut == INVALID_HANDLE_VALUE)
      return false;

    DWORD outMode = 0;
    if (!GetConsoleMode(hOut, &outMode))
      return false;

    DWORD desiredOut = outMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, desiredOut))
      return false;

    HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn != nullptr && hIn != INVALID_HANDLE_VALUE) {
      DWORD inMode = 0;
      if (GetConsoleMode(hIn, &inMode)) {
        SetConsoleMode(hIn, inMode | ENABLE_VIRTUAL_TERMINAL_INPUT);
      }
    }

    return true;
  };

  vtEnabled_ = enableVirtualTerminal();
#else
  vtEnabled_ = true;
#endif

  // Initialize special memory locations
  // LOMEM pointer (locations 105-106)
  pokeMemory(0x0069, lomem_ & 0xFF);
  pokeMemory(0x006A, (lomem_ >> 8) & 0xFF);
  // HIMEM pointer (locations 115-116)
  pokeMemory(0x0073, himem_ & 0xFF);
  pokeMemory(0x0074, (himem_ >> 8) & 0xFF);
  // Cursor vertical position (location 37)
  pokeMemory(0x0025, outputRow_);
}

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
  dataOffsets_.clear();
  dataPointer_ = 0;
  resetOutputPosition();
}

void Interpreter::clearState() {
  variables_.clear();
  forStack_.clear();
  while (!gosubStack_.empty()) {
    gosubStack_.pop();
  }
  dataValues_.clear();
  dataOffsets_.clear();
  dataPointer_ = 0;
  errorHandlerLine_ = -1;
  errorLine_ = -1;
  lastError_.clear();
  resetOutputPosition();
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
  resetOutputPosition();

  // Prepare DATA cache before execution so READ works regardless of control
  // flow.
  dataPointer_ = 0;
  dataValues_.clear();
  dataOffsets_.clear();
  for (const auto &pair : program_) {
    bool recorded = false;
    for (const auto &stmt : pair.second.statements) {
      size_t before = dataValues_.size();
      stmt->collectData(dataValues_);
      if (!recorded && dataValues_.size() > before) {
        dataOffsets_.push_back({pair.first, before});
        recorded = true;
      }
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

      // TRACE output if enabled
      if (tracing_) {
        std::cout << "[" << currentLine_ << "]";
      }

      try {
        for (auto &stmt : programCounter_->second.statements) {
          stmt->execute(this);
          if (!running_ || jumped_)
            break;
          applySpeedDelay();
        }
      } catch (const std::exception &e) {
        if (errorHandlerLine_ >= 0) {
          errorLine_ = currentLine_;
          lastError_ = e.what();
          
          // Store error information in memory locations for PEEK
          // Location 218: error line number (low byte)
          // Location 219: error line number (high byte)
          // Location 222: error code (only set if not already set by handleError)
          pokeMemory(0x00DA, errorLine_ & 0xFF);
          pokeMemory(0x00DB, (errorLine_ >> 8) & 0xFF);
          
          // Only set generic error code if no specific error code was set
          int currentErrorCode = peekMemory(0x00DE);
          if (currentErrorCode == 0) {
            pokeMemory(0x00DE, 16);  // Generic error code
          }
          
          gotoLine(errorHandlerLine_);
          continue;  // Continue executing from error handler
        } else {
          std::cout << "?" << e.what() << " IN LINE " << currentLine_ << "\n";
          running_ = false;
          break;
        }
      }

      if (!jumped_) {
        ++programCounter_;
      }
    }
  } catch (const std::exception &e) {
    // Catch any unhandled exceptions from the main loop
    std::cout << "?" << e.what() << "\n";
    running_ = false;
  }

  running_ = false;
  paused_ = false;
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
    auto trim = [](std::string s) {
      auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
      s.erase(s.begin(), std::find_if(s.begin(), s.end(),
                                      [&](char c) { return !isSpace(c); }));
      s.erase(std::find_if(s.rbegin(), s.rend(),
                           [&](char c) { return !isSpace(c); })
                  .base(),
              s.end());
      return s;
    };

    std::string trimmed = trim(code);
    std::string upperTrim = trimmed;
    std::transform(upperTrim.begin(), upperTrim.end(), upperTrim.begin(),
                   ::toupper);

    auto spacePos = upperTrim.find_first_of(" \t");
    std::string command = (spacePos == std::string::npos)
                              ? upperTrim
                              : upperTrim.substr(0, spacePos);
    std::string args = (spacePos == std::string::npos)
                           ? std::string()
                           : trim(trimmed.substr(spacePos));

    if (command == "RUN") {
      if (args.empty()) {
        run();
      } else {
        try {
          int start = std::stoi(args);
          runFrom(start);
        } catch (...) {
          std::cout << "?SYNTAX ERROR\n";
        }
      }
    } else if (command == "LIST") {
      if (args.empty()) {
        listProgram();
      } else {
        int start = -1;
        int end = -1;
        auto commaPos = args.find(',');
        std::string first =
            (commaPos == std::string::npos) ? args : args.substr(0, commaPos);
        std::string second = (commaPos == std::string::npos)
                                 ? std::string()
                                 : args.substr(commaPos + 1);

        first = trim(first);
        second = trim(second);

        try {
          if (!first.empty()) {
            start = std::stoi(first);
          }
          if (!second.empty()) {
            end = std::stoi(second);
          }
          listProgram(start, end);
        } catch (...) {
          std::cout << "?SYNTAX ERROR\n";
        }
      }
    } else if (command == "NEW") {
      newProgram();
    } else if (command == "CONT") {
      try {
        cont();
      } catch (...) {
        std::cout << "?CAN'T CONTINUE\n";
      }
    } else if (command == "DEL") {
      // DEL start,end
      std::string argsTrim = args;
      auto commaPos = argsTrim.find(',');
      if (commaPos == std::string::npos) {
        std::cout << "?SYNTAX ERROR\n";
      } else {
        try {
          int start = std::stoi(argsTrim.substr(0, commaPos));
          int end = std::stoi(argsTrim.substr(commaPos + 1));
          if (start > end)
            std::swap(start, end);
          for (int ln = start; ln <= end; ++ln) {
            deleteLine(ln);
          }
        } catch (...) {
          std::cout << "?SYNTAX ERROR\n";
        }
      }
    } else if (command.rfind("LOAD", 0) == 0) {
      std::string filename = trim(code.substr(4));
      loadProgram(filename);
    } else if (command.rfind("SAVE", 0) == 0) {
      std::string filename = trim(code.substr(4));
      saveProgram(filename);
    } else if (command.rfind("CHAIN", 0) == 0) {
      std::string filename = trim(code.substr(5));
      chainProgram(filename);
    } else if (command == "-" || trimmed.substr(0, 1) == "-") {
      // DASH command: - filename
      std::string filename = trim(trimmed.substr(1));
      if (!filename.empty()) {
        dashProgram(filename);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "CATALOG" || command == "CAT") {
      catalog();
    } else if (command == "DELETE") {
      if (!args.empty()) {
        deleteFile(args);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "RENAME") {
      // RENAME oldname,newname
      if (!args.empty()) {
        size_t commaPos = args.find(',');
        if (commaPos != std::string::npos) {
          std::string oldName = trim(args.substr(0, commaPos));
          std::string newName = trim(args.substr(commaPos + 1));
          renameFile(oldName, newName);
        } else {
          handleError("SYNTAX ERROR");
        }
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "PREFIX") {
      if (args.empty()) {
        showPrefix();
      } else {
        changePrefix(args);
      }
    } else if (command == "OPEN") {
      if (!args.empty()) {
        openFile(args, "");
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "CLOSE") {
      if (args.empty()) {
        closeAllFiles();
      } else {
        closeFile(args);
      }
    } else if (command == "APPEND") {
      if (!args.empty()) {
        appendFile(args);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "FLUSH") {
      if (args.empty()) {
        handleError("SYNTAX ERROR");
      } else {
        flushFile(args);
      }
    } else if (command == "POSITION") {
      if (!args.empty()) {
        // Parse POSITION filename,Rrecord,Bbyte
        size_t commaPos = args.find(',');
        if (commaPos != std::string::npos) {
          std::string filename = trim(args.substr(0, commaPos));
          std::string rest = args.substr(commaPos + 1);
          int record = 0, byte = 0;
          // Parse R# and B# options
          size_t rPos = rest.find('R');
          size_t bPos = rest.find('B');
          if (rPos != std::string::npos) {
            record = std::stoi(rest.substr(rPos + 1));
          }
          if (bPos != std::string::npos) {
            byte = std::stoi(rest.substr(bPos + 1));
          }
          positionFile(filename, record, byte);
        } else {
          handleError("SYNTAX ERROR");
        }
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "LOCK") {
      if (!args.empty()) {
        lockFile(args);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "UNLOCK") {
      if (!args.empty()) {
        unlockFile(args);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command == "CREATE") {
      if (!args.empty()) {
        createFile(args, "");
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command.rfind("BLOAD", 0) == 0) {
      std::string rest = trim(code.substr(5));  // Skip "BLOAD"
      if (!rest.empty()) {
        // Parse BLOAD filename[,A#]
        size_t commaPos = rest.find(',');
        std::string filename = (commaPos != std::string::npos) ? 
          trim(rest.substr(0, commaPos)) : trim(rest);
        
        // Remove quotes if present
        if (!filename.empty() && filename.front() == '"' && filename.back() == '"') {
          filename = filename.substr(1, filename.length() - 2);
        }
        
        int address = -1;
        if (commaPos != std::string::npos) {
          std::string addrStr = trim(rest.substr(commaPos + 1));
          size_t aPos = addrStr.find('A');
          if (aPos != std::string::npos) {
            address = std::stoi(addrStr.substr(aPos + 1));
          } else {
            address = std::stoi(addrStr);
          }
        }
        bloadFile(filename, address);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command.rfind("BSAVE", 0) == 0) {
      std::string rest = trim(code.substr(5));  // Skip "BSAVE"
      if (!rest.empty()) {
        // Parse BSAVE filename,A#,L#
        size_t firstComma = rest.find(',');
        if (firstComma != std::string::npos) {
          std::string filename = trim(rest.substr(0, firstComma));
          // Remove quotes if present
          if (!filename.empty() && filename.front() == '"' && filename.back() == '"') {
            filename = filename.substr(1, filename.length() - 2);
          }
          
          std::string remaining = rest.substr(firstComma + 1);
          int address = 0, length = 0;
          
          // Parse A# parameter
          size_t secondComma = remaining.find(',');
          if (secondComma != std::string::npos) {
            std::string addrStr = trim(remaining.substr(0, secondComma));
            std::string lenStr = trim(remaining.substr(secondComma + 1));
            
            // Parse A value
            size_t aPos = addrStr.find('A');
            if (aPos != std::string::npos) {
              address = std::stoi(addrStr.substr(aPos + 1));
            } else {
              address = std::stoi(addrStr);
            }
            
            // Parse L value
            size_t lPos = lenStr.find('L');
            if (lPos != std::string::npos) {
              length = std::stoi(lenStr.substr(lPos + 1));
            } else {
              length = std::stoi(lenStr);
            }
          }
          
          if (length > 0) {
            bsaveFile(filename, address, length);
          } else {
            handleError("SYNTAX ERROR");
          }
        } else {
          handleError("SYNTAX ERROR");
        }
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command.rfind("BRUN", 0) == 0) {
      std::string rest = trim(code.substr(4));  // Skip "BRUN"
      if (!rest.empty()) {
        // Parse BRUN filename[,A#]
        size_t commaPos = rest.find(',');
        std::string filename = (commaPos != std::string::npos) ? 
          trim(rest.substr(0, commaPos)) : trim(rest);
        
        // Remove quotes if present
        if (!filename.empty() && filename.front() == '"' && filename.back() == '"') {
          filename = filename.substr(1, filename.length() - 2);
        }
        
        int address = -1;
        if (commaPos != std::string::npos) {
          std::string addrStr = trim(rest.substr(commaPos + 1));
          size_t aPos = addrStr.find('A');
          if (aPos != std::string::npos) {
            address = std::stoi(addrStr.substr(aPos + 1));
          } else {
            address = std::stoi(addrStr);
          }
        }
        brunFile(filename, address);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else if (command.rfind("EXEC", 0) == 0) {
      std::string filename = trim(code.substr(4));
      if (!filename.empty()) {
        execFile(filename);
      } else {
        handleError("SYNTAX ERROR");
      }
    } else {
      // Execute as immediate statement
      immediate_ = true;

      Tokenizer tokenizer;
      std::vector<Token> tokens = tokenizer.tokenize(code);

      Parser parser;
      std::vector<std::shared_ptr<Statement>> statements = parser.parse(tokens);

      for (auto &stmt : statements) {
        stmt->execute(this);
        applySpeedDelay();
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

void Interpreter::stop() {
  // Stop execution but remember where to continue
  paused_ = true;
  continueAfterLine_ = currentLine_;
  running_ = false;
}

void Interpreter::cont() {
  if (!paused_ || program_.empty() || continueAfterLine_ < 0) {
    throw std::runtime_error("CANT CONTINUE");
  }
  // Position to the line after the one that STOPped
  auto it = program_.find(continueAfterLine_);
  if (it == program_.end()) {
    throw std::runtime_error("CANT CONTINUE");
  }
  ++it; // next line
  programCounter_ = it;
  running_ = true;
  immediate_ = false;
  paused_ = false;

  try {
    while (running_ && programCounter_ != program_.end()) {
      currentLine_ = programCounter_->first;
      jumped_ = false;

      // TRACE output if enabled
      if (tracing_) {
        std::cout << "[" << currentLine_ << "]";
      }

      for (auto &stmt : programCounter_->second.statements) {
        stmt->execute(this);
        if (!running_ || jumped_)
          break;
        applySpeedDelay();
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
}

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

void Interpreter::chainProgram(const std::string &filename) {
  try {
    std::string content = readTextFile(filename);
    
    // Clear program but keep variables
    program_.clear();
    dataValues_.clear();
    dataOffsets_.clear();
    dataPointer_ = 0;

    // Load new program
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
    
    // Run the program
    run();
  } catch (const std::exception &e) {
    std::cout << "?" << e.what() << "\n";
  }
}

void Interpreter::dashProgram(const std::string &filename) {
  try {
    std::string content = readTextFile(filename);
    
    // Clear program but keep variables
    program_.clear();
    dataValues_.clear();
    dataOffsets_.clear();
    dataPointer_ = 0;

    // Load new program
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
    
    // Run the program
    run();
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

void Interpreter::execFile(const std::string &filename) {
  try {
    std::string content = readTextFile(filename);
    
    // Execute each line as an immediate command
    std::istringstream iss(content);
    std::string line;
    while (std::getline(iss, line)) {
      if (!line.empty()) {
        // Skip comments and empty lines
        auto trimmed = line;
        auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
        trimmed.erase(trimmed.begin(), std::find_if(trimmed.begin(), trimmed.end(),
                                      [&](char c) { return !isSpace(c); }));
        
        if (!trimmed.empty() && trimmed.substr(0, 3) != "REM") {
          executeImmediate(line);
        }
      }
    }
  } catch (const std::exception &e) {
    std::cout << "?" << e.what() << "\n";
  }
}

void Interpreter::deleteFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!::deleteFile(filename)) {
    handleError("PATH NOT FOUND ERROR");
  }
}

void Interpreter::renameFile(const std::string &oldName, const std::string &newName) {
  if (oldName.empty() || newName.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!::renameFile(oldName, newName)) {
    handleError("I/O ERROR");
  }
}

void Interpreter::showPrefix() {
  std::string prefix = getCurrentPrefix();
  std::cout << prefix << "\n";
}

void Interpreter::changePrefix(const std::string &path) {
  if (path.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!::setPrefix(path)) {
    handleError("PATH NOT FOUND ERROR");
  }
}

void Interpreter::openFile(const std::string &filename, const std::string &options) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    FileAccessMode mode = FileAccessMode::READ;
    // Parse options for mode
    if (options.find("W") != std::string::npos || options.find("WRITE") != std::string::npos) {
      mode = FileAccessMode::WRITE;
    } else if (options.find("A") != std::string::npos || options.find("APPEND") != std::string::npos) {
      mode = FileAccessMode::APPEND;
    }
    FileManager::getInstance().openFile(filename, mode);
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::closeFile(const std::string &filename) {
  try {
    FileManager::getInstance().closeFile(filename);
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::closeAllFiles() {
  FileManager::getInstance().closeAllFiles();
}

void Interpreter::appendFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    FileManager::getInstance().openFile(filename, FileAccessMode::APPEND);
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::flushFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    FileManager::getInstance().flushFile(filename);
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::positionFile(const std::string &filename, int record, int byte) {
  // For now, position is interpreted as byte offset
  // Record numbers are not yet implemented in full ProDOS style
  try {
    auto& fm = FileManager::getInstance();
    // Get handle by filename (this is a simplification)
    // In real ProDOS, you'd use the handle directly
    size_t position = static_cast<size_t>(record * 512 + byte);  // Assume 512-byte records
    handleError("POSITION not fully implemented - use sequential I/O");
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::lockFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!FileManager::getInstance().lockFile(filename)) {
    handleError("I/O ERROR");
  }
}

void Interpreter::unlockFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!FileManager::getInstance().unlockFile(filename)) {
    handleError("I/O ERROR");
  }
}

void Interpreter::createFile(const std::string &filename, const std::string &options) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!FileManager::getInstance().createFile(filename)) {
    handleError("I/O ERROR");
  }
}

void Interpreter::bloadFile(const std::string &filename, int address) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    std::vector<uint8_t> data = FileManager::getInstance().loadBinaryFile(filename, address);
    // Binary data is loaded but not used for anything specific yet
    // In a full Apple II emulator, this would load into memory at the specified address
    std::cout << "BINARY FILE LOADED: " << data.size() << " BYTES\n";
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::bsaveFile(const std::string &filename, int address, int length) {
  if (filename.empty() || length <= 0) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    // Create dummy data for now (in full implementation, would read from memory)
    std::vector<uint8_t> data(length, 0);
    FileManager::getInstance().saveBinaryFile(filename, data, address, length);
    std::cout << "BINARY FILE SAVED: " << length << " BYTES\n";
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::brunFile(const std::string &filename, int address) {
  // BRUN = BLOAD + CALL
  bloadFile(filename, address);
  if (address >= 0) {
    // Would execute machine code at address in full implementation
    callAddress(address);
  }
}

void Interpreter::callAddress(int address) {
  // Handle negative addresses (Apple II convention)
  if (address < 0) {
    address = 0x10000 + address;
  }
  
  // Handle special Apple II CALL addresses
  switch (address) {
    case 0xF308: // -3288: Stack cleanup routine
      // No-op in our implementation
      break;
      
    case 0xF3D2: // -3086: Clear hi-res page to black
      // Would clear graphics buffer in full implementation
      std::cout << "CALL $F3D2: CLEAR HI-RES TO BLACK (STUB)\n";
      break;
      
    case 0xF3D6: // -3082: Clear hi-res to last HPLOT color
      std::cout << "CALL $F3D6: CLEAR HI-RES TO COLOR (STUB)\n";
      break;
      
    case 0xF832: // -1998: BKGND (background color)
      std::cout << "CALL $F832: SET BACKGROUND (STUB)\n";
      break;
      
    case 0xFC42: // -958: Clear from cursor to bottom-right
      std::cout << "CALL $FC42: CLEAR TO BOTTOM (STUB)\n";
      break;
      
    case 0xFC58: // -936: HOME (clear screen, home cursor)
      std::cout << "\033[2J\033[H";  // ANSI clear screen
      outputRow_ = 0;
      pokeMemory(0x0025, outputRow_);
      break;
      
    case 0xFC66: // -922: Line feed
      std::cout << "\n";
      outputRow_++;
      if (outputRow_ >= 24) outputRow_ = 23;
      pokeMemory(0x0025, outputRow_);
      break;
      
    case 0xFC70: // -912: Scroll text window up
      std::cout << "CALL $FC70: SCROLL UP (STUB)\n";
      break;
      
    case 0xFC9C: // -868: CLREOL (clear to end of line)
      std::cout << "\033[K";  // ANSI clear to end of line
      break;
      
    case 0xFF69: // -151: Enter Monitor
      std::cout << "CALL $FF69: MONITOR (NOT IMPLEMENTED)\n";
      break;
      
    case 0x0300: // Common user ML routine location (page 3)
      std::cout << "CALL $0300: USER ROUTINE (STUB)\n";
      break;
      
    case 0x03EA: // Restore ProDOS connection
      std::cout << "CALL $03EA: RESTORE PRODOS (STUB)\n";
      break;
      
    default:
      // Generic machine language call - no-op
      std::cout << "CALL " << formatHexAddress(address) << " (NOT IMPLEMENTED)\n";
      break;
  }
}

namespace {
// Maximum number of array dimensions supported in array files
// 255 is chosen to match typical BASIC array dimension limits while
// being large enough for any practical use case
constexpr size_t kMaxArrayDimensions = 255;

// Helper function to sanitize array name for use as filename
std::string sanitizeArrayName(const std::string &arrayName) {
  std::string filename = arrayName;
  // Remove problematic characters for filenames
  filename.erase(
      std::remove_if(filename.begin(), filename.end(),
                    [](char c) {
                      return c == '$' || c == '%' || c == '/' || c == '\\' ||
                             c == ':' || c == '*' || c == '?' || c == '"' ||
                             c == '<' || c == '>' || c == '|';
                    }),
      filename.end());
  return filename + ".arr";
}
} // namespace

void Interpreter::storeArray(const std::string &arrayName) {
  if (!variables_.hasArray(arrayName)) {
    throw std::runtime_error("UNDEFINED ARRAY ERROR");
  }

  // Use tape if available, otherwise fall back to file
  if (tapeManager_.hasTape()) {
    // Open tape for writing
    tapeManager_.openForWrite();
    
    const auto &dimensions = variables_.getArrayDimensions(arrayName);
    const auto &data = variables_.getArrayData(arrayName);
    
    // Build record data
    std::vector<uint8_t> record;
    
    // Write array name
    uint8_t nameLen = static_cast<uint8_t>(arrayName.length());
    record.push_back(nameLen);
    for (char c : arrayName) {
      record.push_back(static_cast<uint8_t>(c));
    }
    
    // Write dimensions
    uint8_t numDims = static_cast<uint8_t>(dimensions.size());
    record.push_back(numDims);
    for (int dim : dimensions) {
      uint32_t dimVal = static_cast<uint32_t>(dim);
      record.push_back(dimVal & 0xFF);
      record.push_back((dimVal >> 8) & 0xFF);
      record.push_back((dimVal >> 16) & 0xFF);
      record.push_back((dimVal >> 24) & 0xFF);
    }
    
    // Write data count
    uint32_t dataCount = static_cast<uint32_t>(data.size());
    record.push_back(dataCount & 0xFF);
    record.push_back((dataCount >> 8) & 0xFF);
    record.push_back((dataCount >> 16) & 0xFF);
    record.push_back((dataCount >> 24) & 0xFF);
    
    // Write data entries
    for (const auto &entry : data) {
      const auto &indices = entry.first;
      const auto &value = entry.second;
      
      // Write number of indices
      uint8_t numIndices = static_cast<uint8_t>(indices.size());
      record.push_back(numIndices);
      
      // Write indices
      for (int idx : indices) {
        uint32_t idxVal = static_cast<uint32_t>(idx);
        record.push_back(idxVal & 0xFF);
        record.push_back((idxVal >> 8) & 0xFF);
        record.push_back((idxVal >> 16) & 0xFF);
        record.push_back((idxVal >> 24) & 0xFF);
      }
      
      // Write value
      if (value.isString()) {
        record.push_back('S');
        std::string str = value.getString();
        uint32_t strLen = static_cast<uint32_t>(str.length());
        record.push_back(strLen & 0xFF);
        record.push_back((strLen >> 8) & 0xFF);
        record.push_back((strLen >> 16) & 0xFF);
        record.push_back((strLen >> 24) & 0xFF);
        for (char c : str) {
          record.push_back(static_cast<uint8_t>(c));
        }
      } else {
        record.push_back('N');
        double num = value.getNumber();
        uint8_t* numBytes = reinterpret_cast<uint8_t*>(&num);
        for (size_t i = 0; i < sizeof(double); ++i) {
          record.push_back(numBytes[i]);
        }
      }
    }
    
    // Write record to tape
    tapeManager_.writeRecord(record);
    tapeManager_.close();
    
  } else {
    // Fall back to file-based storage
    std::string filename = sanitizeArrayName(arrayName);

    std::ofstream file(filename);
    if (!file) {
      throw std::runtime_error("FILE ERROR");
    }

    const auto &dimensions = variables_.getArrayDimensions(arrayName);
    const auto &data = variables_.getArrayData(arrayName);

    // Write dimensions
    file << dimensions.size();
    for (int dim : dimensions) {
      file << " " << dim;
    }
    file << "\n";

    // Write data
    file << data.size() << "\n";
    for (const auto &entry : data) {
      const auto &indices = entry.first;
      const auto &value = entry.second;
      
      // Write indices
      for (size_t i = 0; i < indices.size(); ++i) {
        if (i > 0) file << ",";
        file << indices[i];
      }
      file << " ";
      
      // Write value
      if (value.isString()) {
        file << "S " << value.getString() << "\n";
      } else {
        file << "N " << value.getNumber() << "\n";
      }
    }
  }
}

void Interpreter::recallArray(const std::string &arrayName) {
  // Use tape if available, otherwise fall back to file
  if (tapeManager_.hasTape()) {
    // Open tape for reading
    tapeManager_.openForRead();
    
    try {
      // Read record from tape
      std::vector<uint8_t> record = tapeManager_.readRecord();
      size_t pos = 0;
      
      // Read array name
      if (pos >= record.size()) {
        throw std::runtime_error("INVALID TAPE FORMAT");
      }
      uint8_t nameLen = record[pos++];
      
      if (pos + nameLen > record.size()) {
        throw std::runtime_error("INVALID TAPE FORMAT");
      }
      std::string recordName(reinterpret_cast<char*>(&record[pos]), nameLen);
      pos += nameLen;
      
      // Verify it matches the requested array name (normalized)
      // For now, we'll just use whatever is on the tape
      
      // Read dimensions
      if (pos >= record.size()) {
        throw std::runtime_error("INVALID TAPE FORMAT");
      }
      uint8_t numDims = record[pos++];
      
      if (numDims == 0 || numDims > kMaxArrayDimensions) {
        throw std::runtime_error("INVALID TAPE FORMAT");
      }
      
      std::vector<int> dimensions(numDims);
      for (size_t i = 0; i < numDims; ++i) {
        if (pos + 4 > record.size()) {
          throw std::runtime_error("INVALID TAPE FORMAT");
        }
        uint32_t dimVal = record[pos] | (record[pos+1] << 8) | 
                         (record[pos+2] << 16) | (record[pos+3] << 24);
        dimensions[i] = static_cast<int>(dimVal);
        pos += 4;
      }
      
      // Read data count
      if (pos + 4 > record.size()) {
        throw std::runtime_error("INVALID TAPE FORMAT");
      }
      uint32_t dataCount = record[pos] | (record[pos+1] << 8) | 
                          (record[pos+2] << 16) | (record[pos+3] << 24);
      pos += 4;
      
      // Read data entries
      std::map<std::vector<int>, Value> data;
      for (size_t i = 0; i < dataCount; ++i) {
        // Read number of indices
        if (pos >= record.size()) {
          throw std::runtime_error("INVALID TAPE FORMAT");
        }
        uint8_t numIndices = record[pos++];
        
        std::vector<int> indices(numIndices);
        for (size_t j = 0; j < numIndices; ++j) {
          if (pos + 4 > record.size()) {
            throw std::runtime_error("INVALID TAPE FORMAT");
          }
          uint32_t idxVal = record[pos] | (record[pos+1] << 8) | 
                           (record[pos+2] << 16) | (record[pos+3] << 24);
          indices[j] = static_cast<int>(idxVal);
          pos += 4;
        }
        
        // Read value type
        if (pos >= record.size()) {
          throw std::runtime_error("INVALID TAPE FORMAT");
        }
        char type = static_cast<char>(record[pos++]);
        
        Value value;
        if (type == 'S') {
          // Read string length
          if (pos + 4 > record.size()) {
            throw std::runtime_error("INVALID TAPE FORMAT");
          }
          uint32_t strLen = record[pos] | (record[pos+1] << 8) | 
                           (record[pos+2] << 16) | (record[pos+3] << 24);
          pos += 4;
          
          if (pos + strLen > record.size()) {
            throw std::runtime_error("INVALID TAPE FORMAT");
          }
          std::string str(reinterpret_cast<char*>(&record[pos]), strLen);
          value = Value(str);
          pos += strLen;
        } else if (type == 'N') {
          // Read double
          if (pos + sizeof(double) > record.size()) {
            throw std::runtime_error("INVALID TAPE FORMAT");
          }
          double num;
          std::memcpy(&num, &record[pos], sizeof(double));
          value = Value(num);
          pos += sizeof(double);
        } else {
          throw std::runtime_error("INVALID TAPE FORMAT");
        }
        
        data[indices] = value;
      }
      
      // Set the array
      variables_.setArrayData(arrayName, dimensions, data);
      
      tapeManager_.close();
      
    } catch (const std::exception &e) {
      tapeManager_.close();
      throw;
    }
    
  } else {
    // Fall back to file-based storage
    std::string filename = sanitizeArrayName(arrayName);

    std::ifstream file(filename);
    if (!file) {
      throw std::runtime_error("PATH NOT FOUND ERROR");
    }

    try {
      // Read dimensions
      size_t numDims;
      file >> numDims;
      if (!file || numDims == 0 || numDims > kMaxArrayDimensions) {
        throw std::runtime_error("INVALID ARRAY FILE FORMAT");
      }
      
      std::vector<int> dimensions(numDims);
      for (size_t i = 0; i < numDims; ++i) {
        file >> dimensions[i];
        if (!file || dimensions[i] < 0) {
          throw std::runtime_error("INVALID ARRAY FILE FORMAT");
        }
      }

      // Read data count
      size_t dataCount;
      file >> dataCount;
      if (!file) {
        throw std::runtime_error("INVALID ARRAY FILE FORMAT");
      }
      file.ignore(); // Skip newline

      std::map<std::vector<int>, Value> data;
      for (size_t i = 0; i < dataCount; ++i) {
        std::string line;
        std::getline(file, line);
        if (!file || line.empty()) {
          throw std::runtime_error("INVALID ARRAY FILE FORMAT");
        }
        
        // Parse indices
        size_t spacePos = line.find(' ');
        if (spacePos == std::string::npos) {
          throw std::runtime_error("INVALID ARRAY FILE FORMAT");
        }
        std::string indicesStr = line.substr(0, spacePos);
        std::string valueStr = line.substr(spacePos + 1);
        
        std::vector<int> indices;
        size_t start = 0;
        while (start < indicesStr.length()) {
          size_t commaPos = indicesStr.find(',', start);
          if (commaPos == std::string::npos) {
            indices.push_back(std::stoi(indicesStr.substr(start)));
            break;
          } else {
            indices.push_back(std::stoi(indicesStr.substr(start, commaPos - start)));
            start = commaPos + 1;
          }
        }
        
        // Parse value
        if (valueStr.length() < 2) {
          throw std::runtime_error("INVALID ARRAY FILE FORMAT");
        }
        char type = valueStr[0];
        std::string valContent = valueStr.substr(2);
        Value value;
        if (type == 'S') {
          value = Value(valContent);
        } else if (type == 'N') {
          value = Value(std::stod(valContent));
        } else {
          throw std::runtime_error("INVALID ARRAY FILE FORMAT");
        }
        
        data[indices] = value;
      }

      // Set the array
      variables_.setArrayData(arrayName, dimensions, data);
    } catch (const std::invalid_argument &) {
      throw std::runtime_error("INVALID ARRAY FILE FORMAT");
    } catch (const std::out_of_range &) {
      throw std::runtime_error("INVALID ARRAY FILE FORMAT");
    }
  }
}

void Interpreter::storeVariables(const std::string &filename) {
  std::ofstream file(filename);
  if (!file) {
    throw std::runtime_error("I/O ERROR");
  }

  // Get all scalar variables from the Variables object
  const auto &numVars = variables_.getAllNumericVariables();
  const auto &strVars = variables_.getAllStringVariables();

  // Write format version
  file << "MSBASIC_VARS_V1\n";

  // Write numeric variables
  file << numVars.size() << "\n";
  for (const auto &pair : numVars) {
    file << pair.first << " " << pair.second << "\n";
  }

  // Write string variables
  file << strVars.size() << "\n";
  for (const auto &pair : strVars) {
    // Escape newlines in strings
    std::string escapedStr = pair.second;
    size_t pos = 0;
    while ((pos = escapedStr.find('\n', pos)) != std::string::npos) {
      escapedStr.replace(pos, 1, "\\n");
      pos += 2;
    }
    file << pair.first << " " << escapedStr << "\n";
  }

  if (!file) {
    throw std::runtime_error("I/O ERROR");
  }
}

void Interpreter::restoreVariables(const std::string &filename) {
  std::ifstream file(filename);
  if (!file) {
    throw std::runtime_error("PATH NOT FOUND ERROR");
  }

  try {
    // Read format version
    std::string version;
    std::getline(file, version);
    if (version != "MSBASIC_VARS_V1") {
      throw std::runtime_error("INVALID VARIABLE FILE FORMAT");
    }

    // Read numeric variables
    size_t numCount;
    file >> numCount;
    file.ignore(); // Skip newline
    
    for (size_t i = 0; i < numCount; ++i) {
      std::string varName;
      double value;
      file >> varName >> value;
      if (!file) {
        throw std::runtime_error("INVALID VARIABLE FILE FORMAT");
      }
      variables_.setVariable(varName, Value(value));
    }
    file.ignore(); // Skip newline after last number

    // Read string variables
    size_t strCount;
    file >> strCount;
    file.ignore(); // Skip newline
    
    for (size_t i = 0; i < strCount; ++i) {
      std::string line;
      std::getline(file, line);
      if (!file || line.empty()) {
        throw std::runtime_error("INVALID VARIABLE FILE FORMAT");
      }
      
      // Parse variable name and value
      size_t spacePos = line.find(' ');
      if (spacePos == std::string::npos) {
        throw std::runtime_error("INVALID VARIABLE FILE FORMAT");
      }
      
      std::string varName = line.substr(0, spacePos);
      std::string value = line.substr(spacePos + 1);
      
      // Unescape newlines
      size_t pos = 0;
      while ((pos = value.find("\\n", pos)) != std::string::npos) {
        value.replace(pos, 2, "\n");
        pos += 1;
      }
      
      variables_.setVariable(varName, Value(value));
    }
  } catch (const std::invalid_argument &) {
    throw std::runtime_error("INVALID VARIABLE FILE FORMAT");
  } catch (const std::out_of_range &) {
    throw std::runtime_error("INVALID VARIABLE FILE FORMAT");
  }
}

void Interpreter::loadShapeTableFromFile(const std::string &filename) {
  // Use tape if available and no filename is provided
  if (filename.empty() && tapeManager_.hasTape()) {
    // Load shape table from tape
    tapeManager_.openForRead();
    
    try {
      // Read record from tape
      std::vector<uint8_t> record = tapeManager_.readRecord();
      
      if (record.empty()) {
        throw std::runtime_error("INVALID SHAPE TABLE FORMAT");
      }
      
      // First byte should be number of shapes
      uint8_t numShapes = record[0];
      if (numShapes == 0 || record.size() < 1 + numShapes * 2) {
        throw std::runtime_error("INVALID SHAPE TABLE FORMAT");
      }
      
      // Store shape table pointer (using a simplified approach)
      pokeMemory(0x00E8, 0);  // Low byte of shape table pointer
      pokeMemory(0x00E9, 0);  // High byte of shape table pointer
      
      tapeManager_.close();
      return;
      
    } catch (const std::exception &e) {
      tapeManager_.close();
      throw std::runtime_error("INVALID SHAPE TABLE FORMAT");
    }
  }
  
  // Load shape table from binary file in Apple II format
  std::ifstream file(filename, std::ios::binary);
  if (!file) {
    throw std::runtime_error("PATH NOT FOUND ERROR");
  }

  try {
    // Read number of shapes (first byte)
    uint8_t numShapes;
    file.read(reinterpret_cast<char*>(&numShapes), 1);
    if (!file || numShapes == 0) {
      throw std::runtime_error("INVALID SHAPE TABLE FORMAT");
    }

    // Read shape table index (2 bytes per shape)
    std::vector<uint16_t> shapeOffsets(numShapes);
    for (size_t i = 0; i < numShapes; ++i) {
      uint8_t lowByte, highByte;
      file.read(reinterpret_cast<char*>(&lowByte), 1);
      file.read(reinterpret_cast<char*>(&highByte), 1);
      if (!file) {
        throw std::runtime_error("INVALID SHAPE TABLE FORMAT");
      }
      shapeOffsets[i] = lowByte | (highByte << 8);
    }

    // Read all remaining data
    std::vector<uint8_t> shapeData;
    char byte;
    while (file.read(&byte, 1)) {
      shapeData.push_back(static_cast<uint8_t>(byte));
    }

    // For now, we'll store the shape table pointer in memory locations 232-233
    // This is a simplified implementation that just validates the file format
    // Full implementation would parse the shape vectors and load them into graphics
    
    // Store shape table pointer (using a simplified approach)
    // In real Apple II, this would be the memory address of the shape table
    // For our implementation, we just acknowledge the file was loaded
    pokeMemory(0x00E8, 0);  // Low byte of shape table pointer
    pokeMemory(0x00E9, 0);  // High byte of shape table pointer
    
    // Note: Full shape table parsing and rendering would require decoding
    // the vector plotting commands in each shape definition. This is a
    // stub implementation that validates file format but doesn't fully
    // parse the shapes.
    
  } catch (const std::exception &e) {
    throw std::runtime_error("INVALID SHAPE TABLE FORMAT");
  }
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

void Interpreter::restoreData(int line) {
  if (line < 0) {
    dataPointer_ = 0;
    return;
  }

  dataPointer_ = dataValues_.size();
  for (const auto &entry : dataOffsets_) {
    if (entry.first >= line) {
      dataPointer_ = entry.second;
      break;
    }
  }
}

void Interpreter::htab(int col1) {
  int targetCol = std::max(0, col1 - 1);
  if (targetCol <= outputColumn_) {
    return;
  }

  int spaces = targetCol - outputColumn_;
  printText(std::string(static_cast<size_t>(spaces), ' '));
}

void Interpreter::vtab(int row1) {
  int targetRow = std::max(0, row1 - 1);
  while (outputRow_ < targetRow) {
    printNewline();
  }
  // Update memory location 37 (cursor vertical position)
  pokeMemory(0x0025, outputRow_);
}

void Interpreter::setInverse(bool on) {
  inverse_ = on;
  updateTextAttributes();
}

void Interpreter::setFlash(bool on) {
  flash_ = on;
  updateTextAttributes();
}

void Interpreter::setNormal() {
  inverse_ = false;
  flash_ = false;
  updateTextAttributes();
}

void Interpreter::updateTextAttributes() {
#ifdef _WIN32
  if (!vtEnabled_) {
    return; // Fall back silently if VT sequences are unavailable.
  }
#endif
  // Apply ANSI styling for inverse/flash; fall back silently if unsupported.
  if (!inverse_ && !flash_) {
    std::cout << "\x1b[0m";
    return;
  }

  std::cout << "\x1b[";
  bool first = true;
  if (inverse_) {
    std::cout << "7";
    first = false;
  }
  if (flash_) {
    if (!first) {
      std::cout << ";";
    }
    std::cout << "5";
  }
  std::cout << "m";
}

void Interpreter::printText(const std::string &text) {
  for (char ch : text) {
    std::cout << ch;
    if (ch == '\a') {
      std::cout << std::flush;
    }
    if (ch == '\n') {
      outputColumn_ = 0;
      outputRow_++;
    } else {
      outputColumn_++;
    }
  }
}

void Interpreter::printNewline() {
  std::cout << "\n";
  outputColumn_ = 0;
  outputRow_++;
  // Update memory location 37 (cursor vertical position)
  pokeMemory(0x0025, outputRow_);
}

void Interpreter::printToNextZone() {
  constexpr int kZoneWidth = 14;
  int nextZoneStart = ((outputColumn_ / kZoneWidth) + 1) * kZoneWidth;
  int spaces = std::max(0, nextZoneStart - outputColumn_);
  if (spaces > 0) {
    printText(std::string(static_cast<size_t>(spaces), ' '));
  }
}

void Interpreter::resetOutputPosition() {
  outputColumn_ = 0;
  outputRow_ = 0;
  setNormal();
}

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

void Interpreter::handleError(const std::string &message, int errorCode) {
  // Store error code in memory location 222 for ProDOS compatibility
  pokeMemory(0x00DE, errorCode);
  throw std::runtime_error(message);
}

void Interpreter::resume() {
  if (errorLine_ < 0) {
    throw std::runtime_error("RESUME WITHOUT ERROR");
  }
  gotoLine(errorLine_);
  errorLine_ = -1;
}

void Interpreter::randomize(double seed) {
  // Initialize random number generator using Float40 for consistency
  Float40 f(seed);
  Float40::setSeed(f);
}

void Interpreter::setSpeedDelay(int delayMs) {
  if (delayMs < 0)
    delayMs = 0;
  if (delayMs > 255)
    delayMs = 255;
  speedDelayMs_ = delayMs;
}

void Interpreter::setOutputDevice(int slot) {
  if (slot < 0)
    slot = 0;
  outputDevice_ = slot;
  
  // Handle special device slots for text mode switching
  if (slot == 0) {
    // PR#0 - switch back to 40-column mode
    setTextMode(TextMode::Text40);
  } else if (slot == 3) {
    // PR#3 - switch to 80-column mode
    setTextMode(TextMode::Text80);
  }
  // Other slots are currently no-ops but we store them for compatibility
}

void Interpreter::setTextMode(TextMode mode) {
  graphicsConfig_.textMode = mode;
  // In the future, this would update the graphics renderer's text display mode
  // For now, we just store the mode for PEEK/POKE compatibility
}

void Interpreter::setInputDevice(int slot) {
  if (slot < 0)
    slot = 0;
  inputDevice_ = slot;
}

void Interpreter::applySpeedDelay() {
  if (speedDelayMs_ <= 0)
    return;
  std::this_thread::sleep_for(std::chrono::milliseconds(speedDelayMs_));
}

void Interpreter::pushWhileLoop(std::shared_ptr<Expression> condition,
                                LineNumber returnLine) {
  WhileLoopInfo info;
  info.condition = condition;
  info.returnLine = returnLine;
  whileStack_.push_back(info);
}

void Interpreter::nextWhileLoop() {
  if (whileStack_.empty()) {
    throw std::runtime_error("WEND WITHOUT WHILE ERROR");
  }

  WhileLoopInfo &loop = whileStack_.back();
  Value val = loop.condition->evaluate(this);

  if (val.getNumber() != 0) {
    // Condition still true, jump back to line after WHILE
    auto it = program_.find(loop.returnLine);
    if (it != program_.end()) {
      ++it;
      programCounter_ = it;
      jumped_ = true;
    }
  } else {
    // Condition false, exit loop
    whileStack_.pop_back();
  }
}

void Interpreter::popGosub() {
  if (gosubStack_.empty()) {
    throw std::runtime_error("POP WITHOUT GOSUB ERROR");
  }
  gosubStack_.pop();
}

void Interpreter::requireGraphicsMode() const {
  if (!graphicsConfig_.isGraphicsEnabled()) {
    // Use error code 255 (general error) for graphics not enabled
    const_cast<Interpreter*>(this)->handleError("GRAPHICS NOT ENABLED ERROR", 255);
  }
}

// Additional ProDOS command implementations

void Interpreter::setPrefix(const std::string &path) {
  if (path.empty()) {
    showPrefix();
  } else {
    changePrefix(path);
  }
}

void Interpreter::catalogFiles(const std::string &path) {
  auto files = listFiles(path);

  std::cout << "\nCAT " << path << "\n\n";
  for (const auto &file : files) {
    if (!file.isDirectory && file.name.length() > 0 && file.name[0] != '.') {
      std::cout << " " << file.name << "\n";
    }
  }
  std::cout << "\n";
}

void Interpreter::chainProgram(const std::string &filename, int startLine) {
  try {
    std::string content = readTextFile(filename);
    
    // Clear program but keep variables
    program_.clear();
    dataValues_.clear();
    dataOffsets_.clear();
    dataPointer_ = 0;

    // Load new program
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
    
    // Run the program from the specified starting line
    if (startLine > 0) {
      runFrom(startLine);
    } else {
      run();
    }
  } catch (const std::exception &e) {
    std::cout << "?" << e.what() << "\n";
  }
}

void Interpreter::dashRun(const std::string &filename) {
  // DASH (-) runs a program without clearing variables
  // It's similar to dashProgram but can also handle binary files
  try {
    // Check file extension to determine type
    if (filename.size() > 4 && filename.substr(filename.size() - 4) == ".bas") {
      // Text BASIC file
      dashProgram(filename);
    } else {
      // Try as binary file
      bloadFile(filename, -1);
      // Could also execute if it's a binary program
    }
  } catch (const std::exception &e) {
    std::cout << "?" << e.what() << "\n";
  }
}

void Interpreter::readFile(const std::string &filename, int record, int byte) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    // Open file for reading
    FileManager::getInstance().openFile(filename, FileAccessMode::READ);
    
    // Position if record/byte specified
    if (record > 0 || byte > 0) {
      size_t position = static_cast<size_t>(record * 512 + byte);
      // TODO: Implement actual file positioning via FileManager handle
      // For now, just open at beginning
      (void)position; // Suppress unused variable warning
    }
    
    std::cout << "FILE OPENED FOR READING: " << filename << "\n";
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::writeFile(const std::string &filename, int record) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  try {
    // Open file for writing
    FileManager::getInstance().openFile(filename, FileAccessMode::WRITE);
    
    // Position if record specified
    if (record > 0) {
      size_t position = static_cast<size_t>(record * 512);
      // TODO: Implement actual file positioning via FileManager handle
      (void)position; // Suppress unused variable warning
    }
    
    std::cout << "FILE OPENED FOR WRITING: " << filename << "\n";
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

void Interpreter::interactive() {
  InteractiveMode interactive;
  interactive.run();
}

void Interpreter::setTapeFile(const std::string &filename) {
  tapeManager_.setTapeFile(filename);
}

std::string Interpreter::getTapeFile() const {
  return tapeManager_.getTapeFile();
}

void Interpreter::changeTapeFile() {
  std::string filename = TapeManager::showFileSelector("Select Tape File");
  if (!filename.empty()) {
    tapeManager_.setTapeFile(filename);
    std::cout << "TAPE CHANGED TO: " << filename << "\n";
  }
}
