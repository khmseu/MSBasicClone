#pragma once

#include "types.h"
#include "parser.h"
#include "variables.h"
#include <map>
#include <vector>
#include <string>
#include <memory>
#include <stack>

class Interpreter {
public:
    Interpreter();
    
    // Program management
    void loadProgram(const std::string& filename);
    void saveProgram(const std::string& filename);
    void newProgram();
    void listProgram(int startLine = -1, int endLine = -1);
    void addLine(LineNumber lineNum, const std::string& text);
    void deleteLine(LineNumber lineNum);
    
    // Execution
    void run();
    void runFrom(LineNumber lineNum);
    void executeImmediate(const std::string& line);
    
    // Interactive mode
    void interactive();
    
    // Variable access (for expressions)
    Variables& getVariables() { return variables_; }
    
    // Get current line number
    LineNumber getCurrentLine() const { return currentLine_; }
    
    // Execution control
    void gotoLine(LineNumber lineNum);
    void gosub(LineNumber lineNum);
    void returnFromGosub();
    void endProgram();
    
    // Data statements
    void addDataValue(const Value& value);
    Value readData();
    void restoreData();
    
    // FOR loops
    void pushForLoop(const std::string& varName, double endValue, double stepValue, LineNumber returnLine);
    bool isInForLoop(const std::string& varName);
    void nextForLoop(const std::string& varName);
    
    // Error handling
    void setErrorHandler(LineNumber lineNum);
    void handleError(const std::string& message);
    void resume();
    
    // File system operations
    void catalog();
    
private:
    Variables variables_;
    std::map<LineNumber, ProgramLine> program_;
    
    // Execution state
    LineNumber currentLine_;
    bool running_;
    bool immediate_;
    bool jumped_;  // Flag to prevent auto-increment after GOTO/GOSUB
    std::map<LineNumber, ProgramLine>::iterator programCounter_;
    
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
    
    // Error handling
    LineNumber errorHandlerLine_;
    std::string lastError_;
    LineNumber errorLine_;
    
    // Helper methods
    void parseLine(const std::string& line, LineNumber& lineNum, std::string& code);
    bool isLineNumber(const std::string& text) const;
};
