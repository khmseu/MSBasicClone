/**
 * @file main.cpp
 * @brief Main entry point for the MSBasic interpreter
 * 
 * This file implements the main() function that handles command-line argument
 * parsing and dispatches execution to either interactive mode (REPL) or script
 * mode (run a BASIC program from a file).
 * 
 * Command-line options:
 * - --graphics: Enable graphics rendering (default)
 * - --no-graphics: Disable graphics (terminal-only mode)
 * - --scale N: Set window scale factor (1-10, default 2)
 * - --tape FILE: Set default tape file for STORE/RECALL/SHLOAD
 * - --tape-hotkey KEY: Set tape change hotkey (default: ESC-T)
 * - --version: Display version information
 * - --help: Display usage information
 * 
 * If a filename is provided, the interpreter runs in script mode, loading and
 * executing the specified BASIC program. Otherwise, it enters interactive mode
 * with the classic Applesoft "]" prompt.
 */

#include "interpreter.h"
#include "interactive.h"
#include "graphics_config.h"
#include "graphics.h"
#include "version.h"
#include <iostream>
#include <string>
#include <cstring>

/**
 * @brief Display command-line usage information
 * @param progName Program name from argv[0]
 */
void printUsage(const char* progName) {
    std::cerr << "Usage: " << progName << " [options] [program.bas]\n"
              << "Options:\n"
              << "  --no-graphics    Terminal-only mode (errors on graphics commands)\n"
              << "  --graphics       Enable graphics mode (default)\n"
              << "  --scale N        Window scale factor (default: 2)\n"
              << "  --tape FILE      Set default tape file\n"
              << "  --tape-hotkey KEY  Set tape change hotkey (default: ESC-T)\n"
              << "  --version        Show version information\n"
              << "  --help           Show this help message\n";
}

/**
 * @brief Main entry point for MSBasic interpreter
 * 
 * Parses command-line arguments, configures the graphics subsystem, and
 * either runs a BASIC program file (script mode) or enters interactive
 * mode (REPL). Returns 0 on success, 1 on error.
 * 
 * @param argc Argument count
 * @param argv Argument vector
 * @return Exit code (0 for success, 1 for error)
 */
int main(int argc, char* argv[]) {
    GraphicsConfig config;
    config.mode = RenderMode::Graphics;  // Default to graphics enabled
    std::string filename;
    std::string tapeFile;
    std::string tapeHotkey = "\x1B" "T";  // ESC-T by default
    bool hasFilename = false;
    
    // Parse command-line arguments
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--no-graphics") == 0) {
            config.mode = RenderMode::NoGraphics;
        } else if (strcmp(argv[i], "--graphics") == 0) {
            config.mode = RenderMode::Graphics;
        } else if (strcmp(argv[i], "--scale") == 0) {
            if (i + 1 < argc) {
                config.scaleFactor = std::atoi(argv[++i]);
                if (config.scaleFactor < 1 || config.scaleFactor > 10) {
                    std::cerr << "Error: Scale factor must be between 1 and 10\n";
                    return 1;
                }
            } else {
                std::cerr << "Error: --scale requires a numeric argument\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--tape") == 0) {
            if (i + 1 < argc) {
                tapeFile = argv[++i];
            } else {
                std::cerr << "Error: --tape requires a filename argument\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--tape-hotkey") == 0) {
            if (i + 1 < argc) {
                tapeHotkey = argv[++i];
            } else {
                std::cerr << "Error: --tape-hotkey requires a key sequence argument\n";
                printUsage(argv[0]);
                return 1;
            }
        } else if (strcmp(argv[i], "--version") == 0) {
            std::cout << "MSBasic " << msbasic::kVersion << "\n";
            return 0;
        } else if (strcmp(argv[i], "--help") == 0) {
            printUsage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            std::cerr << "Error: Unknown option: " << argv[i] << "\n";
            printUsage(argv[0]);
            return 1;
        } else {
            // First non-option argument is the filename
            if (!hasFilename) {
                filename = argv[i];
                hasFilename = true;
            } else {
                std::cerr << "Error: Multiple filenames specified\n";
                printUsage(argv[0]);
                return 1;
            }
        }
    }
    
    try {
        // Initialize graphics system
        graphics().initialize(config);
        
        if (hasFilename) {
            // Script mode - load and run BASIC file
            Interpreter interp(config);
            
            // Set tape options
            if (!tapeFile.empty()) {
                interp.setTapeFile(tapeFile);
            }
            interp.setTapeHotkey(tapeHotkey);
            
            interp.loadProgram(filename);
            interp.run();
            return 0;
        } else {
            // Interactive mode
            InteractiveMode interactive(config);
            interactive.run();
            return 0;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
