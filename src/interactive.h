#pragma once

#include "graphics_config.h"
#include <string>

class InteractiveMode {
public:
    InteractiveMode(const GraphicsConfig& config = GraphicsConfig());
    
    void run();
    
private:
    void printPrompt();
    std::string readLine();
    void printBanner();
    
    GraphicsConfig graphicsConfig_;
};
