#include "interactive.h"
#include "interpreter.h"
#include "version.h"
#include <iostream>
#include <string>

InteractiveMode::InteractiveMode(const GraphicsConfig& config) 
    : graphicsConfig_(config) {}

void InteractiveMode::printBanner() {
  std::cout << "\n";
  std::cout << "APPLESOFT II BASIC CLONE " << msbasic::kVersion << "\n";
  std::cout << "Compatible with Applesoft BASIC\n";
  std::cout << "\n";
}

void InteractiveMode::printPrompt() {
  std::cout << "]";
  std::cout.flush();
}

std::string InteractiveMode::readLine() {
  std::string line;
  std::getline(std::cin, line);
  return line;
}

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
