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

// ProDOS Error Codes (stored in memory location 222)
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

class Interpreter {
public:
  Interpreter(const GraphicsConfig& config = GraphicsConfig());

  // Program management
  void loadProgram(const std::string &filename);
  void saveProgram(const std::string &filename);
  void chainProgram(const std::string &filename);  // Load and run keeping variables
  void dashProgram(const std::string &filename);   // Run program without clearing variables
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

  // Graphics configuration access
  const GraphicsConfig& getGraphicsConfig() const { return graphicsConfig_; }
  bool isGraphicsEnabled() const { return graphicsConfig_.isGraphicsEnabled(); }

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
  
  // Graphics mode checking
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
