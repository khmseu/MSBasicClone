/**
 * @file interactive.h
 * @brief Interactive REPL (Read-Eval-Print Loop) mode
 * 
 * The InteractiveMode class provides a command-line interface for direct
 * interaction with the Applesoft BASIC interpreter. It implements a REPL
 * that accepts both immediate commands and program lines.
 * 
 * Features:
 * - Command prompt display
 * - Line input with history (platform-dependent)
 * - Immediate command execution (no line number)
 * - Program line entry (with line number)
 * - Banner display with version information
 * 
 * Usage:
 * - Lines without numbers execute immediately
 * - Lines with numbers are added to program
 * - Commands like RUN, LIST, NEW, SAVE, LOAD work immediately
 * - Ctrl+C or EXIT command exits REPL
 * 
 * The interactive mode creates its own Interpreter instance configured
 * with the provided GraphicsConfig.
 */

#pragma once

#include "graphics_config.h"
#include <string>

/**
 * @class InteractiveMode
 * @brief Interactive REPL for Applesoft BASIC
 * 
 * Provides command-line interface for direct interaction with the interpreter.
 * Accepts immediate commands and program line entry.
 * 
 * Usage:
 * @code
 * InteractiveMode repl;
 * repl.run();  // Enters REPL until user exits
 * @endcode
 */
class InteractiveMode {
public:
    /**
     * @brief Construct interactive mode with graphics configuration
     * @param config Graphics configuration (default: graphics enabled)
     */
    InteractiveMode(const GraphicsConfig& config = GraphicsConfig());
    
    /**
     * @brief Enter REPL loop
     * 
     * Displays banner, then repeatedly prompts for input and executes
     * commands until user exits (Ctrl+C or EXIT command).
     */
    void run();
    
private:
    /**
     * @brief Display input prompt ("]")
     */
    void printPrompt();
    
    /**
     * @brief Read line of input from user
     * @return Input string (without newline)
     */
    std::string readLine();
    
    /**
     * @brief Display startup banner with version
     */
    void printBanner();
    
    GraphicsConfig graphicsConfig_;
};
