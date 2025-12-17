#pragma once

#include <string>

class InteractiveMode {
public:
    InteractiveMode();
    
    void run();
    
private:
    void printPrompt();
    std::string readLine();
    void printBanner();
};
