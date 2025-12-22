/**
 * @file interpreter.cpp
 * @brief Implementation of the Applesoft BASIC runtime interpreter
 * 
 * This file implements the Interpreter class which executes parsed BASIC
 * programs. It manages program state, control flow, variable storage, I/O,
 * graphics, and all runtime behavior.
 * 
 * Key components:
 * - Program execution (RUN, CONT, immediate mode)
 * - Control flow (GOTO, GOSUB/RETURN, FOR/NEXT, IF/THEN/ELSE)
 * - Variable and array management via Variables class
 * - I/O operations (PRINT, INPUT, GET)
 * - DATA/READ/RESTORE sequence handling
 * - Error handling (ONERR, RESUME)
 * - Graphics integration (GR, HGR, HPLOT, etc.)
 * - File operations (LOAD, SAVE, CATALOG)
 * - Tape operations (STORE, RECALL, SHLOAD)
 * - Memory operations (PEEK, POKE, WAIT)
 * 
 * The interpreter maintains several runtime stacks:
 * - GOSUB stack: Return addresses for subroutines
 * - FOR stack: Loop state (variable, limit, step, return line)
 * - Error handler: ONERR target line and error state
 * 
 * Special features:
 * - TRACE/NOTRACE execution tracing
 * - SPEED command for statement delays (debugging)
 * - Virtual terminal support (Windows and POSIX)
 * - ProDOS-compatible error codes in memory location 222
 */

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

/**
 * @brief Parse a line to separate line number from code
 * 
 * Separates input into line number (if present) and code. Lines without
 * leading numbers are immediate commands (lineNum = -1).
 * 
 * @param line Input line from user
 * @param lineNum Output line number (-1 for immediate)
 * @param code Output code string
 */
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

/**
 * @brief Check if a string is a valid line number
 * @param text String to check
 * @return true if text contains only digits, false otherwise
 */
bool Interpreter::isLineNumber(const std::string &text) const {
  if (text.empty())
    return false;
  for (char c : text) {
    if (!std::isdigit(c))
      return false;
  }
  return true;
}

/**
 * @brief Add or update a program line
 * 
 * If text is empty, deletes the line. Otherwise, tokenizes and parses
 * the line, then stores it in the program map. Lines are kept sorted
 * by line number via std::map.
 * 
 * @param lineNum Line number (0-32767)
 * @param text BASIC code for this line
 */
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

/**
 * @brief Delete a program line
 * 
 * Removes a program line from memory. If the line number doesn't exist,
 * this operation silently succeeds (no error is raised).
 * 
 * Usage in BASIC:
 *   Type just a line number (e.g., "10") to delete line 10
 * 
 * @param lineNum Line number to delete
 */
void Interpreter::deleteLine(LineNumber lineNum) { program_.erase(lineNum); }

/**
 * @brief Clear program and all state (NEW command)
 * 
 * Clears program lines, variables, DATA values, and resets output position.
 * This is equivalent to the NEW command in Applesoft BASIC.
 */
void Interpreter::newProgram() {
  program_.clear();
  variables_.clear();
  dataValues_.clear();
  dataOffsets_.clear();
  dataPointer_ = 0;
  resetOutputPosition();
}

/**
 * @brief Clear variables and control stacks (CLR command)
 * 
 * Clears variables, FOR/NEXT and GOSUB stacks, DATA state, and error
 * handlers. Preserves program lines. This is equivalent to the CLR
 * command in Applesoft BASIC.
 */
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

/**
 * @brief List program lines to stdout (LIST command)
 * 
 * Displays program lines with line numbers. If startLine or endLine are
 * negative, lists all lines. Otherwise, lists lines in the specified range.
 * 
 * @param startLine First line to list (-1 for beginning)
 * @param endLine Last line to list (-1 for end)
 */
void Interpreter::listProgram(int startLine, int endLine) {
  for (const auto &pair : program_) {
    if ((startLine < 0 || pair.first >= startLine) &&
        (endLine < 0 || pair.first <= endLine)) {
      std::cout << pair.first << " " << pair.second.text << "\n";
    }
  }
}

/**
 * @brief Execute program (RUN command implementation)
 * 
 * Starts program execution from the first line.
 * This is a convenience wrapper for runFrom(-1).
 */
void Interpreter::run() { runFrom(-1); }

/**
 * @brief Execute program starting from specified line
 * 
 * This is the main program execution loop implementing the Applesoft BASIC
 * RUN and GOTO behaviors. It handles program flow, DATA caching, error
 * trapping, and tracing.
 * 
 * Execution process:
 * 1. Initialize execution state (running_ flag, immediate_ mode)
 * 2. Build DATA cache by scanning all program lines
 *    - This ensures READ can access DATA regardless of program flow
 *    - dataOffsets_ maps line numbers to positions in dataValues_
 * 3. Set program counter to starting line
 * 4. Execute statements sequentially until:
 *    - END or STOP statement sets running_ = false
 *    - GOTO/GOSUB sets jumped_ flag
 *    - Error occurs
 *    - Program counter reaches end
 * 
 * Error Handling:
 * - If ONERR handler is active (errorHandlerLine_ >= 0):
 *   * Records error line and message
 *   * Stores error info in memory locations 218-219 (line), 222 (code)
 *   * Jumps to error handler line
 * - If no ONERR handler:
 *   * Prints error message and line number
 *   * Stops execution
 * 
 * Special Features:
 * - TRACE mode: Prints [lineNumber] before executing each line
 * - SPEED delay: Optional delay after each statement for debugging
 * - Control flow: jumped_ flag coordinates with GOTO/GOSUB/RETURN
 * 
 * @param lineNum Starting line number, or -1 to start from first line
 */
void Interpreter::runFrom(LineNumber lineNum) {
  running_ = true;
  immediate_ = false;
  resetOutputPosition();

  // Prepare DATA cache before execution so READ works regardless of control
  // flow. This scans all program lines and collects DATA statement values
  // into a linear array (dataValues_) with an index mapping line numbers
  // to positions (dataOffsets_). This allows RESTORE to reposition the
  // data pointer to a specific line's data.
  dataPointer_ = 0;
  dataValues_.clear();
  dataOffsets_.clear();
  for (const auto &pair : program_) {
    bool recorded = false;
    for (const auto &stmt : pair.second.statements) {
      size_t before = dataValues_.size();
      stmt->collectData(dataValues_);
      // Record the offset of the first DATA statement in this line
      if (!recorded && dataValues_.size() > before) {
        dataOffsets_.push_back({pair.first, before});
        recorded = true;
      }
    }
  }

  // Set initial program counter position
  if (lineNum < 0 && !program_.empty()) {
    // Start from first line (RUN with no argument)
    programCounter_ = program_.begin();
  } else {
    // Start from specified line (RUN linenum or GOTO)
    programCounter_ = program_.find(lineNum);
    if (programCounter_ == program_.end()) {
      std::cout << "?UNDEF'D STATEMENT ERROR\n";
      return;
    }
  }

  try {
    // Main execution loop: iterate through program lines
    while (running_ && programCounter_ != program_.end()) {
      currentLine_ = programCounter_->first;
      jumped_ = false;

      // TRACE output if enabled: show line number before execution
      if (tracing_) {
        std::cout << "[" << currentLine_ << "]";
      }

      try {
        // Execute all statements on this line
        for (auto &stmt : programCounter_->second.statements) {
          stmt->execute(this);
          // Stop executing statements if we've jumped or stopped
          if (!running_ || jumped_)
            break;
          // Apply SPEED delay between statements if configured
          applySpeedDelay();
        }
      } catch (const std::exception &e) {
        // Error occurred - check if we have an error handler
        if (errorHandlerLine_ >= 0) {
          // ONERR handler is active - prepare error state
          errorLine_ = currentLine_;
          lastError_ = e.what();
          
          // Store error information in memory locations for PEEK access
          // These locations match Applesoft BASIC conventions:
          // Location 218-219 (0xDA-0xDB): error line number (little-endian)
          // Location 222 (0xDE): error code
          pokeMemory(0x00DA, errorLine_ & 0xFF);
          pokeMemory(0x00DB, (errorLine_ >> 8) & 0xFF);
          
          // Only set generic error code if no specific error code was set
          // by handleError() or other error-generating code
          int currentErrorCode = peekMemory(0x00DE);
          if (currentErrorCode == 0) {
            pokeMemory(0x00DE, 16);  // Generic error code
          }
          
          // Jump to error handler line
          gotoLine(errorHandlerLine_);
          continue;  // Continue executing from error handler
        } else {
          // No error handler - print error and stop
          std::cout << "?" << e.what() << " IN LINE " << currentLine_ << "\n";
          running_ = false;
          break;
        }
      }

      // Advance to next line if we didn't jump
      // (GOTO, GOSUB, or control flow statements set jumped_ flag)
      if (!jumped_) {
        ++programCounter_;
      }
    }
  } catch (const std::exception &e) {
    // Catch any unhandled exceptions from the main execution loop
    // This is a safety net for errors that escape the inner try-catch
    std::cout << "?" << e.what() << "\n";
    running_ = false;
  }

  // Ensure execution state is clean after run completes
  running_ = false;
  paused_ = false;
}

/**
 * @brief Execute an immediate command or add a program line
 * 
 * This is the main entry point for processing user input in interactive mode.
 * It handles two types of input:
 * 
 * 1. Numbered lines (e.g., "10 PRINT "HELLO""):
 *    - Adds or updates a program line
 *    - If code is empty, deletes the line
 * 
 * 2. Immediate commands (no line number):
 *    - Special commands handled directly (RUN, LIST, NEW, etc.)
 *    - Other statements executed immediately in immediate mode
 * 
 * Special Command Processing:
 * - RUN [line]: Start execution from beginning or specified line
 * - LIST [start[,end]]: Display program lines
 * - NEW: Clear program and variables
 * - LOAD, SAVE, CATALOG, DEL: File/program management
 * - CONT: Continue after STOP
 * 
 * The function uses case-insensitive command matching and trims whitespace
 * from arguments. Syntax errors in special commands print an error message
 * but don't stop the interpreter.
 * 
 * @param line Input line from user (may have line number or be immediate)
 */
void Interpreter::executeImmediate(const std::string &line) {
  LineNumber lineNum;
  std::string code;

  // Parse to separate line number (if present) from code
  parseLine(line, lineNum, code);

  if (lineNum >= 0) {
    // Line with number - add to or delete from program
    addLine(lineNum, code);
  } else {
    // Immediate command - execute directly
    
    // Helper lambda to trim whitespace from strings
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

    // Extract command word and arguments
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

    // Handle special immediate-mode commands
    if (command == "RUN") {
      if (args.empty()) {
        run();  // RUN with no argument - start from beginning
      } else {
        // RUN with line number - start from that line
        try {
          int start = std::stoi(args);
          runFrom(start);
        } catch (...) {
          std::cout << "?SYNTAX ERROR\n";
        }
      }
    } else if (command == "LIST") {
      if (args.empty()) {
        listProgram();  // LIST all lines
      } else {
        // LIST with range: LIST start[,end]
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

/**
 * @brief Jump to a program line (GOTO implementation)
 * 
 * Changes the program counter to execute from the specified line number.
 * This is the core implementation of GOTO and is also used by GOSUB,
 * error handlers (ONERR), and ON...GOTO statements.
 * 
 * The function validates that the target line exists in the program and
 * throws "UNDEF'D STATEMENT ERROR" if not found. The jumped_ flag is set
 * to prevent automatic advancement to the next line.
 * 
 * Implementation notes:
 * - Does not affect the GOSUB stack (unlike gosub())
 * - Sets jumped_ flag to override normal sequential execution
 * - Used by: GOTO, error handlers, ON...GOTO, RESTORE (for DATA)
 * 
 * @param lineNum Target line number to jump to
 * @throws std::runtime_error if line number not found in program
 */
/**
 * @brief Jump to a specific line (GOTO implementation)
 * 
 * Changes program execution to the specified line number. This is the
 * core control flow primitive used by GOTO, IF...THEN, and ON...GOTO.
 * 
 * Implementation:
 * - Searches program_ map for target line
 * - Updates programCounter_ to point to that line
 * - Sets jumped_ flag to prevent automatic advancement
 * 
 * The jumped_ flag tells the main execution loop not to advance to the
 * next line automatically, since we've explicitly changed position.
 * 
 * BASIC Usage:
 *   GOTO 100
 *   IF X > 10 THEN GOTO 200
 *   ON N GOTO 100,200,300
 * 
 * @param lineNum Target line number to jump to
 * @throws std::runtime_error if line number not found
 */
void Interpreter::gotoLine(LineNumber lineNum) {
  auto it = program_.find(lineNum);
  if (it == program_.end()) {
    throw std::runtime_error("UNDEF'D STATEMENT ERROR");
  }
  programCounter_ = it;
  jumped_ = true;
}

/**
 * @brief Call a subroutine (GOSUB implementation)
 * 
 * Saves the current line number on the return stack and jumps to the
 * specified subroutine line. The RETURN statement will pop the stack
 * and continue execution after the GOSUB.
 * 
 * Stack structure:
 * - Each GOSUB pushes the current line number (not program counter)
 * - RETURN pops the line and continues from the NEXT line
 * - Nested GOSUBs work through standard stack behavior
 * - Stack is cleared by CLR or program termination
 * 
 * Error conditions:
 * - Target line not found: "UNDEF'D STATEMENT ERROR"
 * - Stack overflow: Limited only by system memory
 * - Unmatched RETURN: "RETURN WITHOUT GOSUB ERROR" (checked in returnFromGosub)
 * 
 * Example:
 *   10 GOSUB 100  ' Push line 10, jump to 100
 *   20 PRINT "BACK"
 *   ...
 *   100 PRINT "SUB"
 *   110 RETURN     ' Pop line 10, continue at line 20
 * 
 * @param lineNum Target subroutine line number
 * @throws std::runtime_error if line number not found
 */
void Interpreter::gosub(LineNumber lineNum) {
  gosubStack_.push(currentLine_);
  auto it = program_.find(lineNum);
  if (it == program_.end()) {
    throw std::runtime_error("UNDEF'D STATEMENT ERROR");
  }
  programCounter_ = it;
  jumped_ = true;
}

/**
 * @brief Return from subroutine (RETURN implementation)
 * 
 * Pops a line number from the GOSUB stack and continues execution at the
 * line immediately following that line. This completes a GOSUB/RETURN pair.
 * 
 * Implementation details:
 * - Pops the return line from the stack
 * - Finds that line in the program
 * - Advances to the NEXT line (incrementing the iterator)
 * - Sets jumped_ flag to prevent further advancement
 * 
 * Special case: If the return line no longer exists (deleted after GOSUB),
 * execution continues from the next available line.
 * 
 * Error handling:
 * - Empty stack: "RETURN WITHOUT GOSUB ERROR"
 * - Missing return line: Advances to next line if available
 * 
 * @throws std::runtime_error if GOSUB stack is empty
 */
void Interpreter::returnFromGosub() {
  if (gosubStack_.empty()) {
    throw std::runtime_error("RETURN WITHOUT GOSUB ERROR");
  }
  LineNumber returnLine = gosubStack_.top();
  gosubStack_.pop();

  // Continue after the GOSUB line
  // Find the line we returned from and advance to the next one
  auto it = program_.find(returnLine);
  if (it != program_.end()) {
    ++it;  // Move to next line
    programCounter_ = it;
    jumped_ = true;
  }
}

/**
 * @brief Stop program execution permanently (END implementation)
 * 
 * Terminates program execution by clearing the running_ flag. Unlike STOP,
 * END does not support continuation with CONT.
 * 
 * Typical usage:
 * - END statement at end of main program
 * - Prevents "fall-through" into subroutines at end of program
 * 
 * Example:
 *   100 PRINT "DONE"
 *   110 END
 *   1000 REM SUBROUTINES FOLLOW
 */
void Interpreter::endProgram() { running_ = false; }

/**
 * @brief Pause program execution (STOP implementation)
 * 
 * Suspends program execution but maintains state for continuation with CONT.
 * Unlike END, STOP allows resuming execution from the next line.
 * 
 * State preservation:
 * - Sets paused_ flag to indicate continuation is possible
 * - Saves current line in continueAfterLine_
 * - Clears running_ flag to exit execution loop
 * - Preserves all variables, stacks, and program state
 * 
 * Used for:
 * - Interactive debugging (insert STOP to examine variables)
 * - Controlled program pauses
 * - Breakpoint simulation
 * 
 * Example:
 *   10 FOR I = 1 TO 10
 *   20 PRINT I
 *   30 IF I = 5 THEN STOP
 *   40 NEXT I
 *   ' Can type CONT to resume from line 40
 */
void Interpreter::stop() {
  // Stop execution but remember where to continue
  paused_ = true;
  continueAfterLine_ = currentLine_;
  running_ = false;
}

/**
 * @brief Continue execution after STOP (CONT implementation)
 * 
 * Resumes program execution from the point where it was stopped by a STOP
 * statement. This allows interactive debugging and controlled program flow.
 * 
 * Continuation process:
 * 1. Validate that program is in paused state
 * 2. Find the line where STOP occurred
 * 3. Advance to the NEXT line (don't re-execute STOP line)
 * 4. Resume normal execution loop
 * 
 * State validation:
 * - Must have paused_ flag set (STOP was executed)
 * - Must have valid continueAfterLine_
 * - Program and continue line must still exist
 * 
 * Error conditions:
 * - Not paused: "CANT CONTINUE"
 * - Program cleared: "CANT CONTINUE"
 * - Continue line deleted: "CANT CONTINUE"
 * 
 * Example session:
 *   ] 10 PRINT "START"
 *   ] 20 STOP
 *   ] 30 PRINT "END"
 *   ] RUN
 *   START
 *   (program stops at line 20)
 *   ] CONT
 *   END
 * 
 * @throws std::runtime_error if continuation is not possible
 */
void Interpreter::cont() {
  if (!paused_ || program_.empty() || continueAfterLine_ < 0) {
    throw std::runtime_error("CANT CONTINUE");
  }
  // Position to the line after the one that STOPped
  auto it = program_.find(continueAfterLine_);
  if (it == program_.end()) {
    throw std::runtime_error("CANT CONTINUE");
  }
  ++it; // Advance to next line
  programCounter_ = it;
  running_ = true;
  immediate_ = false;
  paused_ = false;

  try {
    // Resume execution loop (similar to runFrom but continues where stopped)
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

/**
 * @brief Load a BASIC program from disk (LOAD command implementation)
 * 
 * Loads a BASIC program from a text file, replacing the current program.
 * The file should contain BASIC source code with line numbers.
 * 
 * BASIC Usage:
 *   LOAD "MYPROG.BAS"
 * 
 * File Format:
 *   10 PRINT "HELLO"
 *   20 GOTO 10
 * 
 * Behavior:
 * - Clears current program and variables (like NEW)
 * - Parses each line and adds to program
 * - Silently ignores lines without valid line numbers
 * - On error, prints error message and continues
 * 
 * @param filename Path to BASIC program file
 */
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

/**
 * @brief Save current program to disk (SAVE command implementation)
 * 
 * Saves the current BASIC program to a text file. The file will contain
 * line numbers followed by the BASIC code for each line.
 * 
 * BASIC Usage:
 *   SAVE "MYPROG.BAS"
 * 
 * File Format:
 *   Each line: <line_number><space><code>
 * 
 * Behavior:
 * - Overwrites existing file without warning
 * - Saves lines in sorted order by line number
 * - On error, prints error message but doesn't stop interpreter
 * 
 * @param filename Path where program will be saved
 */
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

/**
 * @brief Chain to another program (CHAIN command implementation)
 * 
 * Loads and runs a new BASIC program while preserving variable values.
 * This allows breaking large programs into multiple files that share data.
 * 
 * BASIC Usage:
 *   CHAIN "PART2.BAS"
 * 
 * Behavior:
 * - Clears current program
 * - Keeps all variable values intact
 * - Loads new program from file
 * - Begins execution at first line of new program
 * - Common variables automatically transfer between programs
 * 
 * Use Cases:
 * - Multi-part adventures/games
 * - Menu systems that load different modules
 * - Programs that exceed memory by splitting into parts
 * 
 * @param filename Path to next program to chain to
 */
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

/**
 * @brief Load and run program without clearing variables (- command implementation)
 * 
 * Implements the "-" (dash) command from Applesoft BASIC which loads a
 * program and runs it immediately while preserving all current variables.
 * This is useful for chaining programs that share data.
 * 
 * Behavior:
 * - Clears the current program listing
 * - Loads new program from file
 * - Preserves all variables and arrays
 * - Automatically runs the loaded program
 * - Clears DATA/READ state (dataValues_, dataOffsets_)
 * 
 * Difference from CHAIN:
 * - CHAIN can specify a starting line and pass values
 * - Dash always starts from first line
 * - Dash is simpler and matches original Apple II behavior
 * 
 * BASIC Usage:
 *   -PROG2  (load and run PROG2 keeping all variables)
 * 
 * @param filename Path to BASIC program file to load and run
 */
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

/**
 * @brief List files in current directory (CATALOG implementation)
 * 
 * Displays a list of all non-hidden files in the current working directory.
 * This implements the CATALOG command from Applesoft BASIC which shows
 * available programs and data files.
 * 
 * Output format:
 * - Header: "CATALOG"
 * - One filename per line with leading space
 * - Hidden files (starting with '.') are not shown
 * - Directories are not shown (files only)
 * 
 * BASIC Usage:
 *   CATALOG  (list all files in current directory)
 * 
 * ProDOS compatibility:
 * - This is a simplified version
 * - Full ProDOS shows file types, sizes, dates
 * - Use PREFIX to change directory before CATALOG
 */
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

/**
 * @brief Execute file as immediate commands (EXEC implementation)
 * 
 * Loads a text file and executes each line as an immediate command, as if
 * typed at the BASIC prompt. This implements the EXEC command from
 * Applesoft BASIC, useful for batch operations and setup scripts.
 * 
 * Processing:
 * - Reads file line by line
 * - Skips empty lines and REM comments
 * - Executes each line via executeImmediate()
 * - Lines can be commands (LIST, RUN, etc.) or program lines with numbers
 * - Errors are caught and displayed but don't stop execution
 * 
 * Common uses:
 * - Automated testing (EXEC TEST.COMMANDS)
 * - Configuration (EXEC SETUP.BAS)
 * - Batch program loading
 * 
 * BASIC Usage:
 *   EXEC COMMANDS.TXT  (execute each line as typed command)
 * 
 * @param filename Path to text file containing commands
 */
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

/**
 * @brief Delete a file (DELETE implementation)
 * 
 * Deletes the specified file from disk. This implements the DELETE command
 * from ProDOS Applesoft BASIC.
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - File not found: "PATH NOT FOUND ERROR"
 * - Permission denied: Handled by filesystem layer
 * 
 * BASIC Usage:
 *   DELETE OLD.DATA
 *   DELETE "TEMP.TXT"
 * 
 * @param filename Name of file to delete
 */
void Interpreter::deleteFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!::deleteFile(filename)) {
    handleError("PATH NOT FOUND ERROR");
  }
}

/**
 * @brief Rename a file (RENAME implementation)
 * 
 * Renames a file from oldName to newName. This implements the RENAME command
 * from ProDOS Applesoft BASIC.
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - Source not found: "I/O ERROR"
 * - Destination exists: "I/O ERROR"
 * - Cross-device rename: Handled by filesystem layer
 * 
 * BASIC Usage:
 *   RENAME OLD.BAS, NEW.BAS
 *   RENAME "DATA1", "DATA2"
 * 
 * @param oldName Current filename
 * @param newName New filename
 */
void Interpreter::renameFile(const std::string &oldName, const std::string &newName) {
  if (oldName.empty() || newName.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!::renameFile(oldName, newName)) {
    handleError("I/O ERROR");
  }
}

/**
 * @brief Display current directory prefix (PREFIX implementation without arguments)
 * 
 * Shows the current working directory (prefix in ProDOS terms). This
 * implements the PREFIX command without arguments from ProDOS Applesoft BASIC.
 * 
 * Output format:
 * - Absolute path to current directory
 * - Unix-style path on Unix systems
 * - Drive letter paths on Windows
 * 
 * BASIC Usage:
 *   PREFIX  (show current directory)
 */
void Interpreter::showPrefix() {
  std::string prefix = getCurrentPrefix();
  std::cout << prefix << "\n";
}

/**
 * @brief Change directory prefix (PREFIX implementation with argument)
 * 
 * Changes the current working directory (prefix in ProDOS terms). All
 * subsequent file operations use this directory as the base. This implements
 * the PREFIX command with argument from ProDOS Applesoft BASIC.
 * 
 * Path handling:
 * - Absolute paths start from root
 * - Relative paths are relative to current prefix
 * - Parent directory: ".." (Unix-style)
 * 
 * Error conditions:
 * - Empty path: "SYNTAX ERROR"
 * - Path not found: "PATH NOT FOUND ERROR"
 * - Not a directory: "PATH NOT FOUND ERROR"
 * 
 * BASIC Usage:
 *   PREFIX /DATA
 *   PREFIX GAMES
 *   PREFIX ..
 * 
 * @param path New directory path (absolute or relative)
 */
void Interpreter::changePrefix(const std::string &path) {
  if (path.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!::setPrefix(path)) {
    handleError("PATH NOT FOUND ERROR");
  }
}

/**
 * @brief Open a file for I/O operations (OPEN implementation)
 * 
 * Opens a file with specified access mode for reading or writing. This
 * implements ProDOS-style file operations from Applesoft BASIC.
 * 
 * Access modes:
 * - READ (default): Open for reading
 * - WRITE (W): Open for writing (truncates existing file)
 * - APPEND (A): Open for appending (writes at end)
 * 
 * File management:
 * - Files are tracked by the FileManager singleton
 * - Multiple files can be open simultaneously
 * - Use CLOSE to release file handles
 * - File position maintained across READ/WRITE operations
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - File not found (READ mode): Varies by implementation
 * - Permission denied: Propagated from filesystem
 * 
 * BASIC Usage:
 *   OPEN "DATA.TXT", READ
 *   OPEN "OUTPUT.TXT", WRITE
 *   OPEN "LOG.TXT", APPEND
 * 
 * @param filename Name of file to open
 * @param options Mode string (READ, WRITE, APPEND, W, A)
 */
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

/**
 * @brief Close an open file (CLOSE implementation)
 * 
 * Closes a previously opened file and releases its file handle. Any buffered
 * data is flushed to disk. This implements the CLOSE command from ProDOS
 * Applesoft BASIC.
 * 
 * Behavior:
 * - Flushes any buffered write data
 * - Releases file handle for reuse
 * - Safe to call on already-closed files (no-op)
 * - File can be reopened after closing
 * 
 * BASIC Usage:
 *   CLOSE "DATA.TXT"
 *   CLOSE  (closes all files)
 * 
 * @param filename Name of file to close
 */
void Interpreter::closeFile(const std::string &filename) {
  try {
    FileManager::getInstance().closeFile(filename);
  } catch (const std::exception &e) {
    handleError(e.what());
  }
}

/**
 * @brief Close all open files
 * 
 * Closes all files that are currently open, flushing any buffered data.
 * This is called by CLOSE without arguments or during program cleanup.
 * 
 * BASIC Usage:
 *   CLOSE  (no filename closes all files)
 */
void Interpreter::closeAllFiles() {
  FileManager::getInstance().closeAllFiles();
}

/**
 * @brief Open file in append mode (APPEND implementation)
 * 
 * Opens a file for appending data at the end. This is a convenience wrapper
 * that calls openFile() with APPEND mode.
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - I/O errors: Propagated from openFile()
 * 
 * BASIC Usage:
 *   APPEND "LOG.TXT"
 * 
 * @param filename Name of file to open for appending
 */
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

/**
 * @brief Flush buffered file data to disk (FLUSH implementation)
 * 
 * Forces any buffered write data for the specified file to be written to
 * disk immediately. Useful for ensuring data persistence before long
 * operations or potential crashes.
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - File not open: Varies by implementation
 * - I/O error: Propagated from filesystem
 * 
 * BASIC Usage:
 *   FLUSH "DATA.TXT"
 * 
 * @param filename Name of file to flush
 */
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

/**
 * @brief Set file position for random access (POSITION implementation - stub)
 * 
 * Sets the file pointer to a specific byte offset for random access I/O.
 * This is a stub implementation - full ProDOS random access is not yet
 * supported. Use sequential I/O instead.
 * 
 * ProDOS Position:
 * - record: Record number (512 bytes per record)
 * - byte: Byte offset within record
 * - position = record * 512 + byte
 * 
 * Current status:
 * - Not fully implemented
 * - Returns error message
 * - Sequential I/O works correctly
 * 
 * BASIC Usage:
 *   POSITION filename, record, byte
 * 
 * @param filename Name of open file
 * @param record Record number (512-byte blocks)
 * @param byte Byte offset within record
 */
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

/**
 * @brief Lock a file to prevent deletion (LOCK implementation)
 * 
 * Marks a file as locked, preventing accidental deletion. This implements
 * the LOCK command from ProDOS Applesoft BASIC.
 * 
 * Platform behavior:
 * - Unix/Linux: Sets read-only permissions
 * - Windows: Sets FILE_ATTRIBUTE_READONLY
 * - macOS: Sets immutable flag where supported
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - File not found: "I/O ERROR"
 * - Permission denied: "I/O ERROR"
 * 
 * BASIC Usage:
 *   LOCK "IMPORTANT.DAT"
 * 
 * @param filename Name of file to lock
 */
void Interpreter::lockFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!FileManager::getInstance().lockFile(filename)) {
    handleError("I/O ERROR");
  }
}

/**
 * @brief Unlock a file to allow modification (UNLOCK implementation)
 * 
 * Removes the lock on a file, allowing it to be modified or deleted. This
 * implements the UNLOCK command from ProDOS Applesoft BASIC.
 * 
 * Platform behavior:
 * - Unix/Linux: Restores write permissions
 * - Windows: Clears FILE_ATTRIBUTE_READONLY
 * - macOS: Clears immutable flag
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - File not found: "I/O ERROR"
 * - Permission denied: "I/O ERROR"
 * 
 * BASIC Usage:
 *   UNLOCK "IMPORTANT.DAT"
 * 
 * @param filename Name of file to unlock
 */
void Interpreter::unlockFile(const std::string &filename) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!FileManager::getInstance().unlockFile(filename)) {
    handleError("I/O ERROR");
  }
}

/**
 * @brief Create a new empty file (CREATE implementation)
 * 
 * Creates a new empty file with the specified name. If the file already
 * exists, behavior depends on the filesystem implementation. This implements
 * the CREATE command from ProDOS Applesoft BASIC.
 * 
 * File creation:
 * - Creates empty file (0 bytes)
 * - Default permissions set by OS
 * - Directory must exist
 * - Parent directory must be writable
 * 
 * Error conditions:
 * - Empty filename: "SYNTAX ERROR"
 * - Directory not found: "I/O ERROR"
 * - Permission denied: "I/O ERROR"
 * - Disk full: "I/O ERROR"
 * 
 * BASIC Usage:
 *   CREATE "NEWFILE.DAT"
 *   CREATE "DATA.TXT", T=TXT  (with type, not yet implemented)
 * 
 * @param filename Name of file to create
 * @param options File type and options (currently ignored)
 */
void Interpreter::createFile(const std::string &filename, const std::string &options) {
  if (filename.empty()) {
    handleError("SYNTAX ERROR");
    return;
  }
  
  if (!FileManager::getInstance().createFile(filename)) {
    handleError("I/O ERROR");
  }
}

/**
 * @brief Load binary file into memory (BLOAD implementation)
 * 
 * Loads a binary file from disk, optionally at a specific memory address.
 * This implements the BLOAD command from Applesoft BASIC, used for loading
 * machine language programs and data files.
 * 
 * BASIC Usage:
 *   BLOAD "PROGRAM.BIN"        (load at default/stored address)
 *   BLOAD "PROGRAM.BIN",A8192  (load at address 8192/$2000)
 *   BLOAD "DATA.BIN",A$4000    (load at hex address $4000)
 * 
 * Address handling:
 * - If address = -1: Uses address stored in file header (if any)
 * - If address >= 0: Loads at specified address, ignoring file header
 * - Negative addresses use Apple II 16-bit wraparound convention
 * 
 * File format compatibility:
 * - ProDOS binary format: 4-byte header (address, length)
 * - Raw binary files: No header, requires explicit address
 * 
 * Current implementation:
 * - Loads file data but doesn't populate emulated memory yet
 * - Full Apple II emulation would copy bytes to memory array
 * - Useful for loading shape tables, fonts, or ML routines
 * 
 * @param filename Path to binary file to load
 * @param address Target memory address, or -1 for file's stored address
 * @throws std::runtime_error on I/O errors or invalid file format
 */
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

/**
 * @brief Save memory region to binary file (BSAVE implementation)
 * 
 * Saves a region of memory to a binary file with ProDOS-compatible format.
 * This implements the BSAVE command from Applesoft BASIC, used for saving
 * machine language programs, screen images, and data arrays.
 * 
 * BASIC Usage:
 *   BSAVE "SCREEN.BIN",A8192,L8192   (save hi-res page 1)
 *   BSAVE "PROG.BIN",A$300,L$100     (save page 3 ML routine)
 *   BSAVE "DATA.BIN",A16384,L4096    (save 4K data block)
 * 
 * Parameters:
 * - filename: Output file path
 * - address: Starting memory address to save from
 * - length: Number of bytes to save
 * 
 * File format:
 * - 4-byte header: 2-byte address (little-endian), 2-byte length
 * - Followed by raw memory contents
 * - Compatible with ProDOS BLOAD command
 * 
 * Common uses in Applesoft:
 * - Save hi-res graphics: BSAVE "PIC",A8192,L8192 (page 1)
 * - Save shape tables: After creating with DRAW/XDRAW
 * - Save ML routines: After POKEing machine code
 * 
 * Current implementation:
 * - Creates dummy data (zeros) instead of reading from memory
 * - Full implementation would copy bytes from emulated memory array
 * 
 * @param filename Path to output binary file
 * @param address Starting memory address
 * @param length Number of bytes to save
 * @throws std::runtime_error on I/O errors or invalid parameters
 */
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

/**
 * @brief Load and execute binary file (BRUN implementation)
 * 
 * Loads a binary file into memory and executes it as machine language code.
 * This implements the BRUN command from Applesoft BASIC. BRUN is equivalent
 * to BLOAD followed by CALL.
 * 
 * BASIC Usage:
 *   BRUN HELLO              (standard DOS 3.3 boot)
 *   BRUN PROGRAM.BIN,A$300  (load and execute at page 3)
 * 
 * Execution sequence:
 * 1. Load binary file via bloadFile()
 * 2. If address specified, call machine code at that address
 * 3. If no address, use file's stored start address
 * 
 * Common uses:
 * - Boot programs: BRUN HELLO
 * - Launch ML utilities: BRUN TOOLKIT
 * - Run assembly programs: BRUN GAME.BIN
 * 
 * Current implementation:
 * - Loads file but doesn't execute actual machine code
 * - Calls callAddress() which handles special Apple II ROM addresses
 * - Full implementation would require CPU emulation
 * 
 * @param filename Path to binary file to load and execute
 * @param address Execution address, or -1 to use file's stored address
 */
void Interpreter::brunFile(const std::string &filename, int address) {
  // BRUN = BLOAD + CALL
  bloadFile(filename, address);
  if (address >= 0) {
    // Would execute machine code at address in full implementation
    callAddress(address);
  }
}

/**
 * @brief Execute machine language at address (CALL implementation)
 * 
 * Simulates calling machine language routines at specific memory addresses.
 * This implements the CALL command from Applesoft BASIC. Since we don't have
 * a full 6502 emulator, we handle known Apple II ROM addresses specially.
 * 
 * BASIC Usage:
 *   CALL -936         (HOME - clear screen, same as $FC58)
 *   CALL 768          (page 3 user ML routine, same as $0300)
 *   CALL -3288        (clean up stack, same as $F308)
 * 
 * Address handling:
 * - Negative addresses use Apple II 16-bit wraparound convention:
 *   CALL -936 becomes $10000 + (-936) = $FC58 (65,368)
 * - This matches Applesoft BASIC address notation
 * 
 * Known ROM addresses (special handling):
 * 
 * Text/Screen Control:
 * - $FC58 (-936): HOME - Clear screen and home cursor
 * - $FC66 (-922): Line feed
 * - $FC70 (-912): Scroll text window up
 * - $FC9C (-868): CLREOL - Clear to end of line
 * - $FC42 (-958): Clear from cursor to bottom-right
 * 
 * Graphics Control:
 * - $F3D2 (-3086): Clear hi-res page to black
 * - $F3D6 (-3082): Clear hi-res to last HPLOT color
 * - $F832 (-1998): BKGND - Set background color
 * 
 * System Control:
 * - $F308 (-3288): Stack cleanup routine
 * - $FF69 (-151): Enter Monitor (not implemented)
 * - $03EA: Restore ProDOS connection
 * 
 * User ML Area:
 * - $0300 (768): Common location for user ML routines (page 3)
 * 
 * Implementation notes:
 * - Known ROM addresses output stub messages or perform equivalent actions
 * - Unknown addresses print "NOT IMPLEMENTED" message
 * - Full implementation would require 6502 CPU emulation
 * - HOME ($FC58) is fully implemented with ANSI clear screen
 * - CLREOL ($FC9C) is fully implemented with ANSI clear to EOL
 * 
 * @param address Memory address to call (may be negative)
 */
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

/**
 * @brief Sanitize array name for use as filename
 * 
 * Converts an array name to a safe filename by removing characters that
 * are problematic on various filesystems (/, \, :, *, ?, ", <, >, |, $, %).
 * Adds ".arr" extension to create a valid array file name.
 * 
 * Examples:
 *   "DATA$" → "DATA.arr"
 *   "VALUES%" → "VALUES.arr"
 *   "MY:ARRAY" → "MYARRAY.arr"
 * 
 * @param arrayName Array name from BASIC program
 * @return Sanitized filename with .arr extension
 */
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

/**
 * @brief Save array to tape or disk (STORE implementation)
 * 
 * Saves an array to cassette tape (if available) or to a disk file. This
 * implements the STORE command from Applesoft BASIC which was originally
 * used for saving arrays to cassette tape.
 * 
 * Storage format (tape):
 * - Array name length (1 byte) + name
 * - Number of dimensions (1 byte)
 * - Dimension sizes (4 bytes each, little-endian)
 * - Data type flag (1 byte: 0=numeric, 1=string)
 * - Array elements (8 bytes for numbers, length+chars for strings)
 * 
 * Storage format (disk):
 * - Same format but saved to <arrayname>.arr file
 * - Filename is sanitized (removes $, %, special chars)
 * 
 * Behavior:
 * - If tape is set (via TAPE command), writes to tape
 * - Otherwise creates .arr file in current directory
 * - Tape maintains position (sequential writes)
 * - Disk creates new file each time (overwrites)
 * 
 * Error conditions:
 * - Array not defined: "UNDEFINED ARRAY ERROR"
 * - I/O error: "FILE ERROR" or tape error
 * 
 * BASIC Usage:
 *   DIM A(100)
 *   STORE A   (save to tape or A.arr)
 * 
 * @param arrayName Name of array to save
 * @throws std::runtime_error if array undefined or I/O error
 */
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
        // Serialize double in native byte order
        // Note: For cross-platform tape compatibility, consider using a portable format
        double num = value.getNumber();
        uint8_t* numBytes = reinterpret_cast<uint8_t*>(&num);
        for (size_t i = 0; i < sizeof(double); ++i) {
          record.push_back(numBytes[i]);
        }
      }
    }
    
    // Write record to tape
    tapeManager_.writeRecord(record);
    // Keep tape open to maintain position
    
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

/**
 * @brief Load array from tape or disk (RECALL implementation)
 * 
 * Loads an array from cassette tape (if available) or from a disk file. This
 * implements the RECALL command from Applesoft BASIC which was originally
 * used for loading arrays from cassette tape.
 * 
 * Loading behavior:
 * - If tape is set, reads next record from tape sequentially
 * - Array name from tape record is used (parameter ignored for tape)
 * - From disk, reads from <arrayname>.arr file
 * - Creates array with DIM if it doesn't exist
 * - Overwrites existing array data if it does exist
 * 
 * File format:
 * - Same format as STORE command
 * - Validates dimensions and data type
 * - Checks for format corruption
 * 
 * Tape vs File:
 * - Tape: Sequential reading, position maintained
 * - File: Random access, reads specific array by name
 * 
 * Error conditions:
 * - File not found (disk): "PATH NOT FOUND ERROR"
 * - Invalid format: "INVALID TAPE FORMAT" or "INVALID ARRAY FILE FORMAT"
 * - Dimension mismatch: Format validation error
 * - I/O error: Propagated from filesystem or tape
 * 
 * BASIC Usage:
 *   RECALL A   (load from tape or A.arr)
 *   RECALL B   (load next array from tape, or B.arr from disk)
 * 
 * @param arrayName Name of array to load (used for disk files)
 * @throws std::runtime_error if file not found or format invalid
 */
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
      
      // For tape format, we use the array name stored in the record
      // rather than requiring it to match the requested name
      // This allows sequential reading of different arrays from tape
      (void)arrayName; // Suppress unused parameter warning
      
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
          // Read double in native byte order
          // Note: Tape files should be used on the same architecture for portability
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
      
      // Keep tape open to maintain position
      
    } catch (const std::exception &e) {
      // Don't close tape on error - let user handle it
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

/**
 * @brief Save all variables to a file (extension of Applesoft functionality)
 * 
 * Saves all scalar variables (numeric and string) to a text file. This is
 * an extension beyond standard Applesoft BASIC, providing persistent
 * variable storage across sessions.
 * 
 * File format:
 * - Header: "MSBASIC_VARS_V1"
 * - Numeric variable count
 * - Each numeric variable: name value
 * - String variable count
 * - Each string variable: name value (with escaped newlines as \n)
 * 
 * Usage:
 * - Save game state or configuration
 * - Checkpoint long-running calculations
 * - Transfer data between programs
 * 
 * Note: Arrays are not included (use STORE for arrays)
 * 
 * Error conditions:
 * - Cannot create file: "I/O ERROR"
 * - Write error: "I/O ERROR"
 * 
 * Example usage:
 *   X = 100: Y$ = "TEST"
 *   CALL ... (implementation-specific call to storeVariables)
 * 
 * @param filename Path to file where variables will be saved
 * @throws std::runtime_error if file cannot be created or written
 */
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

/**
 * @brief Load all variables from a file (extension of Applesoft functionality)
 * 
 * Loads all scalar variables (numeric and string) from a text file previously
 * saved with storeVariables(). This is an extension beyond standard Applesoft
 * BASIC, providing persistent variable storage across sessions.
 * 
 * File format:
 * - Must match format from storeVariables()
 * - Validates version header "MSBASIC_VARS_V1"
 * - Reads numeric variables first, then string variables
 * - Escaped newlines (\n) are unescaped
 * 
 * Behavior:
 * - Existing variables with same names are overwritten
 * - Variables not in file are preserved
 * - Arrays are not affected (use RECALL for arrays)
 * 
 * Error conditions:
 * - File not found: "PATH NOT FOUND ERROR"
 * - Wrong format: "INVALID VARIABLE FILE FORMAT"
 * - Corrupt data: "INVALID VARIABLE FILE FORMAT"
 * 
 * Example usage:
 *   CALL ... (implementation-specific call to restoreVariables)
 *   PRINT X, Y$  (variables restored from file)
 * 
 * @param filename Path to file containing saved variables
 * @throws std::runtime_error if file not found or format invalid
 */
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

/**
 * @brief Load shape table from file or tape (SHLOAD implementation)
 * 
 * Loads a shape table containing vector graphics shapes for use with DRAW
 * and XDRAW commands. This implements the SHLOAD command from Applesoft
 * BASIC which originally loaded shape tables from cassette tape.
 * 
 * Shape table format:
 * - Shape count (1 byte)
 * - For each shape:
 *   * Point count (2 bytes, little-endian)
 *   * Points as (x,y) pairs (4 bytes each: 2 for x, 2 for y, little-endian)
 * 
 * Loading behavior:
 * - If filename is empty and tape is set: reads from tape
 * - If filename provided: reads from disk file
 * - Shape table pointer stored in memory locations 0xE8-0xE9
 * - Shapes are numbered starting from 1
 * - Shape 0 is invalid/undefined
 * 
 * Usage with graphics:
 *   SHLOAD "SHAPES.SHP"
 *   DRAW 1 AT 100,100  (draw shape 1)
 *   XDRAW 2 AT 50,50   (draw shape 2 in XOR mode)
 * 
 * Error conditions:
 * - File not found: "PATH NOT FOUND ERROR"
 * - Invalid format: "INVALID SHAPE TABLE FORMAT"
 * - Corrupt data: "INVALID SHAPE TABLE FORMAT"
 * - Too many shapes: Handled by format limits
 * 
 * @param filename Path to shape table file (empty string for tape)
 * @throws std::runtime_error if file not found or format invalid
 */
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
      
      // Keep tape open to maintain position
      return;
      
    } catch (const std::exception &e) {
      // Don't close tape on error
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

/**
 * @brief Add a data value to the DATA cache
 * 
 * Called during program initialization to build the data cache. This is used
 * internally by the DATA statement collection process.
 * 
 * @param value Value to add to data cache
 */
void Interpreter::addDataValue(const Value &value) {
  dataValues_.push_back(value);
}

/**
 * @brief Read next value from DATA statements (READ implementation)
 * 
 * Returns the next value from the DATA cache and advances the data pointer.
 * The DATA cache is built before program execution by scanning all DATA
 * statements, regardless of where they appear in program flow.
 * 
 * BASIC Usage:
 *   READ A, B$, C
 * 
 * Behavior:
 * - Returns next unread DATA value
 * - Advances data pointer
 * - Throws "OUT OF DATA ERROR" if no more data available
 * - Works across multiple DATA statements
 * - Not affected by program control flow (GOTO, IF, etc.)
 * 
 * @return Next value from DATA statements
 * @throws std::runtime_error if no more data available
 */
Value Interpreter::readData() {
  if (dataPointer_ >= dataValues_.size()) {
    throw std::runtime_error("OUT OF DATA ERROR");
  }
  return dataValues_[dataPointer_++];
}

/**
 * @brief Reset DATA pointer (RESTORE implementation)
 * 
 * Resets the data pointer to the beginning of DATA statements or to a
 * specific line number. This allows re-reading data values.
 * 
 * BASIC Usage:
 *   RESTORE         (reset to beginning)
 *   RESTORE 1000    (reset to DATA on line 1000 or after)
 * 
 * Behavior:
 * - RESTORE with no argument: reset to first DATA value
 * - RESTORE line: reset to first DATA value at or after specified line
 * - If line has no DATA, pointer set to end (OUT OF DATA on next READ)
 * 
 * Algorithm:
 * - Uses dataOffsets_ map that links line numbers to positions in dataValues_
 * - Searches for first line >= target line that has DATA
 * - Sets dataPointer_ to corresponding position in dataValues_ array
 * 
 * @param line Line number to restore to, or -1 for beginning
 */
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

/**
 * @brief Move cursor to horizontal column (HTAB implementation)
 * 
 * Moves the cursor to the specified column by printing spaces. This implements
 * the HTAB (horizontal tab) command from Applesoft BASIC.
 * 
 * Behavior:
 * - Column numbers are 1-based (HTAB 1 = leftmost column)
 * - If target column is left of or at current position, does nothing
 * - If target column is right of current position, prints spaces to reach it
 * - Does not wrap to next line if target exceeds line width
 * 
 * BASIC Usage:
 *   HTAB 10: PRINT "TEXT"  (print starting at column 10)
 *   HTAB 1                 (return to left margin)
 * 
 * @param col1 Target column (1-based, 1 = leftmost)
 */
void Interpreter::htab(int col1) {
  int targetCol = std::max(0, col1 - 1);
  if (targetCol <= outputColumn_) {
    return;
  }

  int spaces = targetCol - outputColumn_;
  printText(std::string(static_cast<size_t>(spaces), ' '));
}

/**
 * @brief Move cursor to vertical row (VTAB implementation)
 * 
 * Moves the cursor to the specified row by printing newlines. This implements
 * the VTAB (vertical tab) command from Applesoft BASIC.
 * 
 * Behavior:
 * - Row numbers are 1-based (VTAB 1 = top row)
 * - If target row is above current position, does nothing
 * - If target row is below current position, prints newlines to reach it
 * - Updates memory location 0x0025 (37) with current cursor row
 * 
 * BASIC Usage:
 *   VTAB 10: PRINT "TEXT"  (print starting at row 10)
 *   VTAB 1                 (return to top of screen)
 *   HTAB 1: VTAB 1         (home cursor to top-left)
 * 
 * Memory Update:
 * - Location 37 ($25): Cursor vertical position (0-based internally)
 * 
 * @param row1 Target row (1-based, 1 = top)
 */
void Interpreter::vtab(int row1) {
  int targetRow = std::max(0, row1 - 1);
  while (outputRow_ < targetRow) {
    printNewline();
  }
  // Update memory location 37 (cursor vertical position)
  pokeMemory(0x0025, outputRow_);
}

/**
 * @brief Enable inverse video mode (INVERSE implementation)
 * 
 * Enables inverse (reverse) video text output where foreground and background
 * colors are swapped. This implements the INVERSE command from Applesoft BASIC.
 * 
 * Platform support:
 * - Uses ANSI escape sequence \x1b[7m for inverse video
 * - Windows: Requires virtual terminal support (enabled in constructor)
 * - Unix/Linux: Works on any terminal supporting ANSI codes
 * 
 * BASIC Usage:
 *   INVERSE: PRINT "HIGHLIGHTED"
 *   NORMAL: PRINT "REGULAR"
 * 
 * @param on true to enable inverse video, false to disable
 */
void Interpreter::setInverse(bool on) {
  inverse_ = on;
  updateTextAttributes();
}

/**
 * @brief Enable flashing text mode (FLASH implementation)
 * 
 * Enables blinking/flashing text output. This implements the FLASH command
 * from Applesoft BASIC.
 * 
 * Platform support:
 * - Uses ANSI escape sequence \x1b[5m for blinking text
 * - Windows: Requires virtual terminal support (enabled in constructor)
 * - Unix/Linux: Works on terminals supporting ANSI codes
 * - Note: Not all terminal emulators support blinking text
 * 
 * BASIC Usage:
 *   FLASH: PRINT "BLINKING"
 *   NORMAL: PRINT "STEADY"
 * 
 * @param on true to enable flashing, false to disable
 */
void Interpreter::setFlash(bool on) {
  flash_ = on;
  updateTextAttributes();
}

/**
 * @brief Reset text to normal mode (NORMAL implementation)
 * 
 * Disables both inverse and flash text modes, returning to normal text output.
 * This implements the NORMAL command from Applesoft BASIC.
 * 
 * Behavior:
 * - Clears inverse_ flag
 * - Clears flash_ flag
 * - Sends ANSI reset sequence \x1b[0m to terminal
 * 
 * BASIC Usage:
 *   INVERSE: PRINT "REVERSE"
 *   NORMAL: PRINT "REGULAR"
 */
void Interpreter::setNormal() {
  inverse_ = false;
  flash_ = false;
  updateTextAttributes();
}

/**
 * @brief Update terminal text attributes (internal helper)
 * 
 * Sends ANSI escape sequences to the terminal to apply current text
 * attributes (inverse and/or flash). This is called internally after
 * attribute state changes.
 * 
 * ANSI sequences used:
 * - \x1b[0m: Reset all attributes
 * - \x1b[7m: Inverse (reverse video)
 * - \x1b[5m: Blink (flash)
 * - \x1b[7;5m: Both inverse and blink
 * 
 * Platform considerations:
 * - Windows: Checks vtEnabled_ flag, falls back silently if unavailable
 * - Unix/Linux: Always attempts to send sequences
 * - Terminal support varies: not all terminals support blinking
 */
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

/**
 * @brief Print text to output (internal helper)
 * 
 * Outputs text character by character, tracking cursor position for HTAB/VTAB.
 * This is the low-level output primitive used by PRINT and other output commands.
 * 
 * Character handling:
 * - Bell character (\a): Flushes output immediately for audible feedback
 * - Newline (\n): Resets column to 0, increments row
 * - Other characters: Increments column position
 * 
 * Position tracking:
 * - outputColumn_: Current horizontal position (0-based)
 * - outputRow_: Current vertical position (0-based)
 * - These are used by HTAB, VTAB, and POS() function
 * 
 * @param text Text string to output
 */
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

/**
 * @brief Print newline and update cursor tracking
 * 
 * Outputs a newline character and updates cursor position tracking. Also
 * updates memory location 0x0025 (37) with the new cursor row for
 * compatibility with Applesoft BASIC PEEK operations.
 * 
 * Cursor updates:
 * - outputColumn_ set to 0 (start of new line)
 * - outputRow_ incremented
 * - Memory location 37: Updated with new row value
 * 
 * Used by:
 * - PRINT with no trailing semicolon/comma
 * - VTAB to advance to target row
 * - Automatic line wrapping (if implemented)
 */
void Interpreter::printNewline() {
  std::cout << "\n";
  outputColumn_ = 0;
  outputRow_++;
  // Update memory location 37 (cursor vertical position)
  pokeMemory(0x0025, outputRow_);
}

/**
 * @brief Advance to next print zone (comma separator handling)
 * 
 * Implements the comma separator behavior in PRINT statements. Advances the
 * cursor to the next 14-column zone boundary. This matches Applesoft BASIC's
 * columnar output formatting.
 * 
 * Zone layout:
 * - Zones are 14 characters wide
 * - Zone 1: columns 0-13
 * - Zone 2: columns 14-27
 * - Zone 3: columns 28-41
 * - Zone 4: columns 42-55
 * - Zone 5: columns 56-69
 * 
 * Behavior:
 * - Calculates next zone boundary from current position
 * - Prints spaces to reach that boundary
 * - If already at boundary, advances to next zone
 * 
 * BASIC Usage:
 *   PRINT "NAME", "AGE", "CITY"
 *   PRINT A, B, C  (values in columns)
 * 
 * @note Zone width of 14 matches Applesoft BASIC convention
 */
void Interpreter::printToNextZone() {
  constexpr int kZoneWidth = 14;
  int nextZoneStart = ((outputColumn_ / kZoneWidth) + 1) * kZoneWidth;
  int spaces = std::max(0, nextZoneStart - outputColumn_);
  if (spaces > 0) {
    printText(std::string(static_cast<size_t>(spaces), ' '));
  }
}

/**
 * @brief Reset output cursor position to home
 * 
 * Resets the output cursor tracking to the top-left position (0,0) and
 * clears all text attributes (inverse, flash). Called at the start of
 * program execution and by screen-clearing operations.
 * 
 * Resets:
 * - outputColumn_ = 0 (leftmost column)
 * - outputRow_ = 0 (top row)
 * - Text attributes to NORMAL (no inverse, no flash)
 * 
 * Used by:
 * - Program startup (RUN command)
 * - Screen clear operations
 * - Initialize immediate mode
 */
void Interpreter::resetOutputPosition() {
  outputColumn_ = 0;
  outputRow_ = 0;
  setNormal();
}

/**
 * @brief Push a new FOR loop onto the loop stack (FOR statement)
 * 
 * Creates and pushes a FOR loop control structure when a FOR statement
 * is executed. The loop stack maintains nested loop state for proper
 * NEXT processing.
 * 
 * Implementation details:
 * - Stores loop variable name, end value, step value, and return line
 * - Multiple nested FOR loops are supported through the stack
 * - Each FOR creates a new stack entry even if variable name reused
 * - NEXT processes loops from most recent (top of stack) backwards
 * 
 * BASIC Usage:
 *   FOR I = 1 TO 10 STEP 2
 *   FOR J = 1 TO 5
 *     ...
 *   NEXT J
 *   NEXT I
 * 
 * Stack behavior:
 * - Nested loops push multiple entries
 * - NEXT pops completed loops
 * - Jumping out of loops leaves entries (cleaned by CLR or END)
 * 
 * @param varName Loop control variable name
 * @param endValue Final value for loop (TO value)
 * @param stepValue Increment per iteration (STEP value, default 1)
 * @param returnLine Line number where FOR appears (for loop restart)
 */
void Interpreter::pushForLoop(const std::string &varName, double endValue,
                              double stepValue, LineNumber returnLine) {
  ForLoopInfo info;
  info.varName = varName;
  info.endValue = endValue;
  info.stepValue = stepValue;
  info.returnLine = returnLine;
  forStack_.push_back(info);
}

/**
 * @brief Check if a variable is currently a FOR loop control variable
 * 
 * Searches the FOR loop stack to determine if the specified variable
 * is currently controlling an active FOR loop. Used to prevent certain
 * operations that would corrupt loop state.
 * 
 * @param varName Variable name to check
 * @return true if variable is a FOR loop control variable, false otherwise
 */
bool Interpreter::isInForLoop(const std::string &varName) {
  for (const auto &loop : forStack_) {
    if (loop.varName == varName)
      return true;
  }
  return false;
}

/**
 * @brief Process NEXT statement for loop iteration
 * 
 * Handles the NEXT statement which increments the loop variable and either
 * continues the loop (jumping back to statement after FOR) or terminates it
 * (popping from stack and continuing forward).
 * 
 * Algorithm:
 * 1. Find matching FOR loop (search backwards through stack)
 * 2. Increment loop variable by STEP value
 * 3. Check termination condition:
 *    - Positive STEP: continue if var <= end
 *    - Negative STEP: continue if var >= end
 * 4. If continuing: jump to line after FOR statement
 * 5. If done: pop loop from stack and continue forward
 * 
 * BASIC Usage:
 *   FOR I = 1 TO 10 STEP 2
 *     PRINT I
 *   NEXT I
 *   
 *   FOR I = 10 TO 1 STEP -1  (counting down)
 *     PRINT I
 *   NEXT I
 *   
 *   NEXT  (NEXT without variable matches most recent FOR)
 * 
 * Error handling:
 * - If no matching FOR found: "NEXT WITHOUT FOR ERROR"
 * - Variable name matching uses normalized names (2-char significance)
 * - Empty varName matches most recent loop (allows bare NEXT)
 * 
 * @param varName Loop variable name (empty string matches most recent loop)
 * @throws std::runtime_error if no matching FOR loop found
 */
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

/**
 * @brief Set error handler line (ONERR GOTO implementation)
 * 
 * Enables error handling by setting the target line number for the ONERR
 * handler. When an error occurs during program execution, control will
 * jump to this line instead of terminating the program.
 * 
 * BASIC Usage:
 *   ONERR GOTO 9000  (enable error handler at line 9000)
 * 
 * Error handler behavior:
 * - When error occurs, errorLine_ is set to the line where error happened
 * - lastError_ contains the error message text
 * - Memory location 218-219 contains error line number
 * - Memory location 222 contains ProDOS error code
 * - RESUME statement returns to the error line
 * 
 * @param lineNum Target line number for error handler
 */
void Interpreter::setErrorHandler(LineNumber lineNum) {
  errorHandlerLine_ = lineNum;
}

/**
 * @brief Throw a runtime error
 * 
 * Simple error throwing mechanism. Throws std::runtime_error with the
 * provided message. If an ONERR handler is active, the main execution
 * loop will catch this and transfer control to the handler.
 * 
 * @param message Error message text (e.g., "SYNTAX ERROR", "OUT OF DATA ERROR")
 * @throws std::runtime_error always
 */
void Interpreter::handleError(const std::string &message) {
  throw std::runtime_error(message);
}

/**
 * @brief Throw a runtime error with ProDOS error code
 * 
 * Extended error throwing that includes a ProDOS-compatible error code.
 * The error code is stored in memory location 222 ($DE) for compatibility
 * with Applesoft BASIC programs that check error codes via PEEK.
 * 
 * Common ProDOS error codes:
 * - 4: PATH NOT FOUND
 * - 5: VOLUME NOT FOUND
 * - 7: DUPLICATE FILENAME
 * - 45: FILE LOCKED
 * - 46: VOLUME LOCKED
 * - 52: NOT A PRODOS DISK
 * - 56: BAD BUFFER ADDRESS
 * 
 * @param message Error message text
 * @param errorCode ProDOS error code to store in memory location 222
 * @throws std::runtime_error always
 */
void Interpreter::handleError(const std::string &message, int errorCode) {
  // Store error code in memory location 222 for ProDOS compatibility
  pokeMemory(0x00DE, errorCode);
  throw std::runtime_error(message);
}

/**
 * @brief Resume execution after error (RESUME implementation)
 * 
 * Returns execution to the line where an error occurred. Used in ONERR
 * error handlers to retry the failed operation or continue from the
 * error point.
 * 
 * BASIC Usage:
 *   1000 ONERR GOTO 9000
 *   ...
 *   9000 REM ERROR HANDLER
 *   9010 PRINT "ERROR: ";PEEK(218)+PEEK(219)*256
 *   9020 RESUME
 * 
 * Behavior:
 * - Jumps to the line number stored in errorLine_
 * - Clears errorLine_ to prevent multiple RESUMEs
 * - If no error active: "RESUME WITHOUT ERROR"
 * 
 * Note: Unlike some BASIC dialects, this implementation does not support
 * RESUME NEXT (continue after error line). Only RESUME (retry error line).
 * 
 * @throws std::runtime_error if no active error to resume from
 */
void Interpreter::resume() {
  if (errorLine_ < 0) {
    throw std::runtime_error("RESUME WITHOUT ERROR");
  }
  gotoLine(errorLine_);
  errorLine_ = -1;
}

/**
 * @brief Initialize random number generator (implicit RND behavior)
 * 
 * Seeds the random number generator used by RND() function. Uses Float40
 * for consistency with Applesoft BASIC's floating-point arithmetic.
 * 
 * Applesoft BASIC behavior:
 * - RND(1) or RND(n > 0): next random number (doesn't reseed)
 * - RND(0): repeat last random number
 * - RND(n < 0): reseed with |n| and return first number
 * 
 * This function is called when negative argument to RND is used.
 * 
 * @param seed Random seed value (typically negative when explicitly seeding)
 */
void Interpreter::randomize(double seed) {
  // Initialize random number generator using Float40 for consistency
  Float40 f(seed);
  Float40::setSeed(f);
}

/**
 * @brief Set statement execution delay (SPEED implementation)
 * 
 * Sets a delay in milliseconds that is applied after each statement
 * execution. This implements the SPEED command from Applesoft BASIC,
 * useful for debugging or creating animated effects.
 * 
 * BASIC Usage:
 *   SPEED=255  (maximum delay, very slow)
 *   SPEED=50   (moderate delay)
 *   SPEED=0    (no delay, full speed - default)
 * 
 * Delay range:
 * - Minimum: 0 ms (no delay)
 * - Maximum: 255 ms
 * - Values outside range are clamped
 * 
 * The delay is applied via applySpeedDelay() called after each statement
 * in the main execution loop.
 * 
 * @param delayMs Delay in milliseconds (0-255)
 */
void Interpreter::setSpeedDelay(int delayMs) {
  if (delayMs < 0)
    delayMs = 0;
  if (delayMs > 255)
    delayMs = 255;
  speedDelayMs_ = delayMs;
}

/**
 * @brief Set output device slot (PR# implementation)
 * 
 * Redirects output to a specific peripheral slot. This implements the PR#
 * (print) command from Applesoft BASIC which was used to select output
 * devices like printers and 80-column cards.
 * 
 * Apple II slot assignments:
 * - Slot 0: Internal screen (40-column text mode)
 * - Slot 1-2: Serial/printer interfaces
 * - Slot 3: 80-column card (switches to Text80 mode)
 * - Slot 4-7: Other peripherals
 * 
 * Current implementation:
 * - Slot 0: 40-column text mode
 * - Slot 3: 80-column text mode
 * - Other slots: Stored but no special behavior
 * 
 * BASIC Usage:
 *   PR#3  (switch to 80-column mode)
 *   PR#0  (switch back to 40-column mode)
 *   PR#1  (redirect to printer - not fully implemented)
 * 
 * @param slot Peripheral slot number (0-7)
 */
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

/**
 * @brief Set text display mode (40 or 80 column)
 * 
 * Changes the text display mode between 40-column and 80-column modes.
 * This is typically called by PR# command when switching between slot 0
 * (40-column) and slot 3 (80-column card).
 * 
 * Text modes:
 * - Text40: Standard 40-column mode (classic Apple II)
 * - Text80: 80-column mode (requires 80-column card in real hardware)
 * 
 * BASIC Usage:
 *   PR#3  (enables 80-column mode via setTextMode)
 *   PR#0  (returns to 40-column mode via setTextMode)
 * 
 * Future expansion:
 * - Could update graphics renderer's text display
 * - Could adjust line width for text wrapping
 * - Mode is stored for PEEK/POKE compatibility
 * 
 * @param mode Text display mode (Text40 or Text80)
 */
void Interpreter::setTextMode(TextMode mode) {
  graphicsConfig_.textMode = mode;
  // In the future, this would update the graphics renderer's text display mode
  // For now, we just store the mode for PEEK/POKE compatibility
}

/**
 * @brief Set input device slot (IN# implementation)
 * 
 * Redirects input from a specific peripheral slot. This implements the IN#
 * command from Applesoft BASIC which was used to select input devices like
 * modems and serial interfaces.
 * 
 * Apple II slot assignments:
 * - Slot 0: Keyboard (default input)
 * - Slot 1-2: Serial/modem interfaces
 * - Slot 3-7: Other peripherals
 * 
 * Current implementation:
 * - All slots: Stored but no special behavior yet
 * - Keyboard input always active
 * - Future: Could redirect GET/INPUT to serial ports
 * 
 * BASIC Usage:
 *   IN#0  (keyboard input - default)
 *   IN#2  (serial port input - not fully implemented)
 * 
 * @param slot Peripheral slot number (0-7)
 */
void Interpreter::setInputDevice(int slot) {
  if (slot < 0)
    slot = 0;
  inputDevice_ = slot;
}

/**
 * @brief Apply SPEED delay between statements (internal helper)
 * 
 * Sleeps for the configured speed delay time if delay is active.
 * Called by the main execution loop after each statement to implement
 * the SPEED command's slow-motion execution feature.
 * 
 * Only delays if speedDelayMs_ > 0. This is an internal helper called
 * automatically during program execution.
 */
void Interpreter::applySpeedDelay() {
  if (speedDelayMs_ <= 0)
    return;
  std::this_thread::sleep_for(std::chrono::milliseconds(speedDelayMs_));
}

/**
 * @brief Push WHILE loop state onto stack (WHILE implementation helper)
 * 
 * Records the current WHILE loop's condition and return point when entering
 * a WHILE loop. This information is used by WEND to determine if the loop
 * should continue or exit.
 * 
 * WHILE loop structure:
 * - WHILE condition: Evaluate condition, push state if true
 * - ... loop body ...
 * - WEND: Pop state, check condition, loop or exit
 * 
 * Stack entry contains:
 * - condition: Expression to re-evaluate at WEND
 * - returnLine: Line number to jump to if continuing
 * 
 * BASIC Usage:
 *   10 X = 0
 *   20 WHILE X < 10
 *   30   PRINT X
 *   40   X = X + 1
 *   50 WEND
 * 
 * @param condition Expression to evaluate for loop continuation
 * @param returnLine Line number to return to when continuing loop
 */
void Interpreter::pushWhileLoop(std::shared_ptr<Expression> condition,
                                LineNumber returnLine) {
  WhileLoopInfo info;
  info.condition = condition;
  info.returnLine = returnLine;
  whileStack_.push_back(info);
}

/**
 * @brief Handle WEND statement (WHILE loop terminator)
 * 
 * Checks the WHILE loop condition and either continues the loop or exits.
 * This implements the WEND statement which closes a WHILE loop.
 * 
 * Behavior:
 * - Pops WHILE loop info from stack
 * - Re-evaluates the loop condition
 * - If true: Jumps back to line after WHILE statement
 * - If false: Continues to next statement (exits loop)
 * 
 * Error conditions:
 * - No matching WHILE: "WEND WITHOUT WHILE ERROR"
 * 
 * BASIC Usage:
 *   WHILE X < 10
 *     X = X + 1
 *   WEND  (check condition and loop or continue)
 * 
 * @throws std::runtime_error if no matching WHILE found
 */
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

/**
 * @brief Pop GOSUB return address from stack (POP implementation)
 * 
 * Removes the top entry from the GOSUB stack without returning. This
 * implements the POP command from Applesoft BASIC which allows exiting
 * from a subroutine without using RETURN.
 * 
 * Use cases:
 * - Exit subroutine early without returning
 * - Clear return address after error in subroutine
 * - Clean up stack when changing program flow
 * 
 * Error conditions:
 * - Empty stack: "POP WITHOUT GOSUB ERROR"
 * 
 * BASIC Usage:
 *   100 GOSUB 1000
 *   110 PRINT "BACK"
 *   ...
 *   1000 IF X < 0 THEN POP: GOTO 100  (restart instead of return)
 *   1010 RETURN
 * 
 * @throws std::runtime_error if GOSUB stack is empty
 */
void Interpreter::popGosub() {
  if (gosubStack_.empty()) {
    throw std::runtime_error("POP WITHOUT GOSUB ERROR");
  }
  gosubStack_.pop();
}

/**
 * @brief Check if graphics mode is enabled (internal helper)
 * 
 * Validates that graphics operations are allowed. Throws an error if
 * graphics are disabled (--no-graphics flag). This is called by all
 * graphics commands (GR, HGR, HPLOT, DRAW, etc.).
 * 
 * Error handling:
 * - If graphics disabled: "GRAPHICS NOT ENABLED ERROR" with code 255
 * - Error can be caught by ONERR handler
 * - Allows programs to test for graphics availability
 * 
 * Usage context:
 * - Called automatically by graphics commands
 * - Not directly accessible from BASIC
 * - Const method that modifies error state via const_cast (necessary
 *   for error reporting from const methods)
 * 
 * @throws Uses handleError() which may throw or jump to ONERR handler
 */
void Interpreter::requireGraphicsMode() const {
  if (!graphicsConfig_.isGraphicsEnabled()) {
    // Use error code 255 (general error) for graphics not enabled
    const_cast<Interpreter*>(this)->handleError("GRAPHICS NOT ENABLED ERROR", 255);
  }
}

// Additional ProDOS command implementations

/**
 * @brief Set or show directory prefix (internal PREFIX helper)
 * 
 * Internal implementation of the PREFIX command. Dispatches to showPrefix()
 * if path is empty, otherwise changes to the specified directory.
 * 
 * This is a convenience wrapper that handles the dual behavior of PREFIX:
 * - PREFIX (no argument): Show current directory
 * - PREFIX path: Change to directory
 * 
 * @param path New directory path (empty to show current)
 */
void Interpreter::setPrefix(const std::string &path) {
  if (path.empty()) {
    showPrefix();
  } else {
    changePrefix(path);
  }
}

/**
 * @brief List files in specified directory (internal CATALOG helper)
 * 
 * Lists all non-hidden files in the specified directory. This is similar to
 * catalog() but allows specifying a directory path instead of using the
 * current directory.
 * 
 * Output format:
 * - Header: "CAT <path>"
 * - One filename per line with leading space
 * - Hidden files (starting with '.') are excluded
 * - Directories are excluded (files only)
 * 
 * Used internally by ProDOS-style CATALOG commands that specify a path.
 * 
 * @param path Directory path to list
 */
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
