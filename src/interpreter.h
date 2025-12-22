/**
 * @file interpreter.h
 * @brief Runtime execution engine for Applesoft BASIC programs
 * 
 * The Interpreter class is the core runtime engine that executes parsed BASIC
 * programs. It manages program state, variable storage, control flow, I/O
 * operations, and graphics modes while maintaining Applesoft BASIC compatibility.
 * 
 * Key responsibilities:
 * - Program lifecycle (NEW, LOAD, SAVE, RUN, LIST)
 * - Statement execution and control flow (GOTO, GOSUB, FOR/NEXT, IF/THEN)
 * - Variable and array management via Variables class
 * - DATA/READ/RESTORE sequence management
 * - I/O operations (PRINT, INPUT, GET)
 * - Graphics mode control (GR, HGR, HPLOT, COLOR, etc.)
 * - ProDOS-compatible file operations
 * - Error handling with Applesoft error messages
 * - TRACE/NOTRACE execution tracing
 * - SPEED command for statement delays
 * 
 * The interpreter maintains program lines in sorted order and supports both
 * immediate mode (direct command execution) and program mode (running stored
 * programs). It handles the full Applesoft BASIC statement set including
 * DEF FN user-defined functions, ONERR error trapping, and CHAIN/- program
 * loading.
 */

#pragma once

#include "functions.h"
#include "parser.h"
#include "types.h"
#include "variables.h"
#include "graphics_config.h"
#include "tape_manager.h"
#include <map>
#include <memory>
#include <stack>
#include <string>
#include <vector>

/**
 * @namespace ErrorCode
 * @brief ProDOS-compatible error codes stored in memory location 222
 * 
 * These error codes match ProDOS conventions and are used for file I/O
 * and system operations. The error code is accessible via PEEK(222) in
 * BASIC programs for error handling.
 */
namespace ErrorCode {
  constexpr int RANGE_ERROR = 2;
  constexpr int NO_DEVICE_CONNECTED = 3;
  constexpr int WRITE_PROTECTED = 4;
  constexpr int END_OF_DATA = 5;
  constexpr int PATH_NOT_FOUND = 6;
  constexpr int IO_ERROR = 8;
  constexpr int DISK_FULL = 9;
  constexpr int FILE_LOCKED = 10;
  constexpr int INVALID_OPTION = 11;
  constexpr int NO_BUFFERS_AVAILABLE = 12;
  constexpr int FILE_TYPE_MISMATCH = 13;
  constexpr int PROGRAM_TOO_LARGE = 14;
  constexpr int NOT_DIRECT_COMMAND = 15;
  constexpr int DIRECTORY_FULL = 17;
  constexpr int FILE_NOT_OPEN = 18;
  constexpr int DUPLICATE_FILENAME = 19;
  constexpr int FILE_BUSY = 20;
  constexpr int FILES_STILL_OPEN = 21;
}

/**
 * @class Interpreter
 * @brief Runtime execution engine for Applesoft BASIC programs
 * 
 * The Interpreter executes parsed BASIC statements and manages all runtime state.
 * It maintains program lines, variable storage, control flow stacks, and handles
 * all I/O operations while preserving Applesoft BASIC semantics.
 * 
 * Program execution flow:
 * - NEW: Clear program and variables
 * - LOAD: Load program from disk (clears variables)
 * - CHAIN: Load program keeping variables
 * - -: Run program without clearing variables
 * - RUN: Execute from first line or specified line
 * - CONT: Continue after STOP
 * 
 * Control flow:
 * - Maintains GOSUB return stack
 * - Tracks FOR/NEXT loop state
 * - Handles GOTO/GOSUB target line lookup
 * - Supports ONERR error trapping with RESUME
 * 
 * Memory model:
 * - LOMEM/HIMEM define variable storage bounds
 * - PEEK/POKE/WAIT access memory addresses
 * - Memory location 222 holds ProDOS error codes
 * - Memory locations for text/graphics configuration
 * 
 * Graphics:
 * - TEXT, GR (40×48 low-res), HGR/HGR2 (280×192 high-res)
 * - COLOR n (low-res), HCOLOR=n (high-res)
 * - PLOT x,y / HPLOT x,y
 * - DRAW/XDRAW shape commands
 * - ROTATE/SCALE shape transformations
 */
class Interpreter {
public:
  /**
   * @brief Construct interpreter with optional graphics configuration
   * @param config Graphics rendering configuration (default: graphics enabled)
   */
  Interpreter(const GraphicsConfig& config = GraphicsConfig());

  // Program management
  
  /**
   * @brief Load BASIC program from file
   * @param filename Path to .bas or tokenized BASIC file
   * @throws RuntimeError on file I/O errors
   * 
   * Clears current program and variables, then loads the specified file.
   * Supports both plain text (.bas) and tokenized BASIC formats.
   */
  void loadProgram(const std::string &filename);
  
  /**
   * @brief Save BASIC program to text file
   * @param filename Destination path for .bas file
   * @throws RuntimeError on file I/O errors
   */
  void saveProgram(const std::string &filename);
  
  /**
   * @brief Load and run program keeping current variables (CHAIN)
   * @param filename Path to program file
   * 
   * Loads the specified program and begins execution while preserving
   * all variable values. Used for multi-part programs.
   */
  void chainProgram(const std::string &filename);
  
  /**
   * @brief Run program without clearing variables (-)
   * @param filename Path to program file
   * 
   * Similar to CHAIN but doesn't automatically start execution.
   */
  void dashProgram(const std::string &filename);
  
  /**
   * @brief Clear program and variables (NEW)
   * 
   * Resets interpreter to clean state: clears all program lines,
   * variables, arrays, DEF FN definitions, and DATA pointers.
   */
  void newProgram();
  
  /**
   * @brief List program lines to output (LIST)
   * @param startLine Starting line number (-1 for first line)
   * @param endLine Ending line number (-1 for last line)
   * 
   * Displays program lines in the specified range. With no arguments,
   * lists entire program.
   */
  void listProgram(int startLine = -1, int endLine = -1);
  
  /**
   * @brief Add or replace program line
   * @param lineNum Line number (0-32767)
   * @param text Line content (statements)
   * 
   * If line exists, replaces it; otherwise inserts new line in sorted order.
   * Empty text deletes the line.
   */
  void addLine(LineNumber lineNum, const std::string &text);
  
  /**
   * @brief Delete program line
   * @param lineNum Line number to remove
   */
  void deleteLine(LineNumber lineNum);

  // Execution
  
  /**
   * @brief Run program from first line (RUN)
   * 
   * Initializes runtime state (clears variables, resets DATA pointer,
   * clears stacks) and begins execution from first line.
   */
  void run();
  
  /**
   * @brief Run program from specified line (RUN n)
   * @param lineNum Starting line number
   * @throws RuntimeError if line not found
   */
  void runFrom(LineNumber lineNum);
  
  /**
   * @brief Execute single line without line number (immediate mode)
   * @param line BASIC statement(s) to execute
   * 
   * Parses and executes the line immediately. Used for direct commands
   * and REPL interaction.
   */
  void executeImmediate(const std::string &line);

  // Interactive mode
  
  /**
   * @brief Enter interactive REPL mode
   * 
   * Displays prompt and processes user input until exit.
   * Deprecated: Use InteractiveMode class instead.
   */
  void interactive();

  // Variable access (for expressions)
  
  /**
   * @brief Get variable storage for expression evaluation
   * @return Reference to Variables instance
   */
  Variables &getVariables() { return variables_; }

  // Graphics configuration access
  
  /**
   * @brief Get graphics configuration
   * @return Reference to GraphicsConfig
   */
  const GraphicsConfig& getGraphicsConfig() const { return graphicsConfig_; }
  
  /**
   * @brief Check if graphics mode is enabled
   * @return true if graphics operations are allowed
   */
  bool isGraphicsEnabled() const { return graphicsConfig_.isGraphicsEnabled(); }

  // Get current line number
  
  /**
   * @brief Get currently executing line number
   * @return Current line number or -1 if in immediate mode
   */
  LineNumber getCurrentLine() const { return currentLine_; }

  // Execution control
  
  /**
   * @brief Jump to specified line (GOTO)
   * @param lineNum Target line number
   * @throws RuntimeError if line not found
   */
  void gotoLine(LineNumber lineNum);
  
  /**
   * @brief Call subroutine at line (GOSUB)
   * @param lineNum Subroutine line number
   * @throws RuntimeError if line not found or stack overflow
   * 
   * Pushes return address on GOSUB stack and jumps to target line.
   */
  void gosub(LineNumber lineNum);
  
  /**
   * @brief Return from subroutine (RETURN)
   * @throws RuntimeError if GOSUB stack is empty
   * 
   * Pops return address from stack and continues execution.
   */
  void returnFromGosub();
  
  /**
   * @brief End program execution (END)
   * 
   * Halts program normally. State preserved for CONT.
   */
  void endProgram();
  
  /**
   * @brief Stop program execution (STOP)
   * 
   * Pauses program with "BREAK IN line" message. Can resume with CONT.
   */
  void stop();
  
  /**
   * @brief Continue after STOP (CONT)
   * @throws RuntimeError if no program stopped
   */
  void cont();

  // Output helpers
  
  /**
   * @brief Move cursor to column (HTAB)
   * @param col1 Column number (1-based, 1-40 for 40-col, 1-80 for 80-col)
   */
  void htab(int col1);
  
  /**
   * @brief Move cursor to row (VTAB)
   * @param row1 Row number (1-based, 1-24)
   */
  void vtab(int row1);
  
  /**
   * @brief Enable inverse video mode (INVERSE)
   * @param on true to enable, false to disable
   */
  void setInverse(bool on);
  
  /**
   * @brief Enable flashing text mode (FLASH)
   * @param on true to enable, false to disable
   */
  void setFlash(bool on);
  
  /**
   * @brief Return to normal video mode (NORMAL)
   */
  void setNormal();
  
  /**
   * @brief Output text string
   * @param text String to print
   * 
   * Respects current text attributes (inverse, flash) and handles
   * TAB/SPC positioning.
   */
  void printText(const std::string &text);
  
  /**
   * @brief Output newline
   */
  void printNewline();
  
  /**
   * @brief Tab to next print zone (14-character zones)
   */
  void printToNextZone();
  
  /**
   * @brief Reset output position to column 1
   */
  void resetOutputPosition();
  
  // Graphics mode checking
  
  /**
   * @brief Verify graphics operations are allowed
   * @throws RuntimeError if graphics disabled
   */
  void requireGraphicsMode() const;
  
  // Text mode switching (PR#3 for 80-col, PR#0 for 40-col)
  void setTextMode(TextMode mode);
  TextMode getTextMode() const { return graphicsConfig_.textMode; }

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
  void handleError(const std::string &message, int errorCode);
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
    // Update memory locations 115-116 (HIMEM pointer)
    pokeMemory(115, himem_ & 0xFF);
    pokeMemory(116, (himem_ >> 8) & 0xFF);
  }
  void setLomem(int val) {
    lomem_ = val;
    setMemoryBounds(lomem_, himem_);
    // Update memory locations 105-106 (LOMEM pointer)
    pokeMemory(105, lomem_ & 0xFF);
    pokeMemory(106, (lomem_ >> 8) & 0xFF);
  }
  int getHimem() const { return himem_; }
  int getLomem() const { return lomem_; }

  // GOSUB stack manipulation (for POP)
  void popGosub();

  // File system operations
  void catalog();
  void catalogFiles(const std::string &path = ".");  // CAT with optional path
  void execFile(const std::string &filename);  // Execute commands from text file
  
  // ProDOS commands
  void deleteFile(const std::string &filename);
  void renameFile(const std::string &oldName, const std::string &newName);
  void showPrefix();
  void setPrefix(const std::string &path);  // Set prefix or show if empty
  void changePrefix(const std::string &path);
  void chainProgram(const std::string &filename, int startLine);  // Load and run with starting line
  void dashRun(const std::string &filename);  // Run without clearing variables
  
  // ProDOS file I/O commands
  void openFile(const std::string &filename, const std::string &options);
  void closeFile(const std::string &filename);
  void closeAllFiles();
  void appendFile(const std::string &filename);
  void flushFile(const std::string &filename);
  void positionFile(const std::string &filename, int record, int byte);
  void readFile(const std::string &filename, int record, int byte);  // ProDOS READ
  void writeFile(const std::string &filename, int record);  // ProDOS WRITE
  void lockFile(const std::string &filename);
  void unlockFile(const std::string &filename);
  void createFile(const std::string &filename, const std::string &options);
  
  // Binary file operations
  void bloadFile(const std::string &filename, int address);
  void bsaveFile(const std::string &filename, int address, int length);
  void brunFile(const std::string &filename, int address);
  
  // Apple II system calls
  void callAddress(int address);

  // Array persistence
  void storeArray(const std::string &arrayName);
  void recallArray(const std::string &arrayName);

  // Variable persistence (ProDOS STORE/RESTORE)
  void storeVariables(const std::string &filename);
  void restoreVariables(const std::string &filename);

  // Shape table loading
  void loadShapeTableFromFile(const std::string &filename);

  // Tape operations
  void setTapeFile(const std::string &filename);
  std::string getTapeFile() const;
  void changeTapeFile();  // Show file selector and change tape
  void setTapeHotkey(const std::string &hotkey) { tapeHotkey_ = hotkey; }
  std::string getTapeHotkey() const { return tapeHotkey_; }

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

  // Graphics configuration
  GraphicsConfig graphicsConfig_;

  // Tape manager
  TapeManager tapeManager_;
  std::string tapeHotkey_ = "\x1B" "T"; // ESC-T by default

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
