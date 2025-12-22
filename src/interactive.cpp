/**
 * @file interactive.cpp
 * @brief Interactive REPL (Read-Eval-Print Loop) mode implementation
 * 
 * This file implements the InteractiveMode class which provides the classic
 * Applesoft BASIC interactive prompt ("]") where users can enter immediate
 * commands or program lines.
 * 
 * Interactive mode features:
 * - Classic "]" prompt matching Applesoft BASIC
 * - Immediate command execution (without line numbers)
 * - Program line entry and editing (with line numbers)
 * - Error reporting with "?" prefix
 * - EOF handling for graceful exit
 */

#include "interactive.h"
#include "interpreter.h"
#include "version.h"
#include <iostream>
#include <string>

/**
 * @brief Construct an InteractiveMode instance
 * @param config Graphics configuration to use for the interpreter
 */
InteractiveMode::InteractiveMode(const GraphicsConfig& config) 
    : graphicsConfig_(config) {}

/**
 * @brief Display the startup banner
 * 
 * Shows the program name, version, and compatibility message when
 * entering interactive mode.
 */
void InteractiveMode::printBanner() {
  std::cout << "\n";
  std::cout << "APPLESOFT II BASIC CLONE " << msbasic::kVersion << "\n";
  std::cout << "Compatible with Applesoft BASIC\n";
  std::cout << "\n";
}

/**
 * @brief Display the interactive prompt
 * 
 * Outputs the classic Applesoft "]" prompt and flushes stdout to ensure
 * it's visible before waiting for input.
 */
void InteractiveMode::printPrompt() {
  std::cout << "]";
  std::cout.flush();
}

/**
 * @brief Read a line of input from stdin
 * @return The line entered by the user
 */
std::string InteractiveMode::readLine() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}

/**
 * @brief Run the interactive REPL loop
 * 
 * Displays the banner, creates an interpreter, and enters the main loop:
 * 1. Display prompt
 * 2. Read input line
 * 3. Execute line (immediate command or program line entry)
 * 4. Display errors with "?" prefix
 * 5. Repeat until EOF (Ctrl+D on Unix, Ctrl+Z on Windows)
 */
void InteractiveMode::run() {
  printBanner();

  Interpreter interp(graphicsConfig_);

  while (true) {
    printPrompt();
    std::string line = readLine();

    if (std::cin.eof()) {
      break;
    }

    if (line.empty()) {
      continue;
    }

    try {
      interp.executeImmediate(line);
    } catch (const std::exception &e) {
      std::cout << "?" << e.what() << "\n";
    }
  }
}
