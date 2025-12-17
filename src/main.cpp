#include "interpreter.h"
#include "interactive.h"
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    if (argc > 1) {
        // Script mode - load and run BASIC file
        std::string filename = argv[1];
        
        try {
            Interpreter interp;
            interp.loadProgram(filename);
            interp.run();
            return 0;
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else {
        // Interactive mode
        InteractiveMode interactive;
        interactive.run();
        return 0;
    }
}
