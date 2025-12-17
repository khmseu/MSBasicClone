#include "interactive.h"
#include "interpreter.h"
#include <iostream>
#include <string>

InteractiveMode::InteractiveMode() {}

void InteractiveMode::printBanner() {
    std::cout << "\n";
    std::cout << "APPLESOFT II BASIC CLONE v1.0\n";
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
    
    Interpreter interp;
    
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
        } catch (const std::exception& e) {
            std::cout << "?" << e.what() << "\n";
        }
    }
}
