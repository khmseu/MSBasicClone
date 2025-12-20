#include "graphics_renderer.h"
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#endif

#ifdef HAVE_RAYLIB
#include <raylib.h>
#endif

GraphicsRenderer::GraphicsRenderer(const GraphicsConfig& config)
    : config_(config), initialized_(false), windowWidth_(0), windowHeight_(0)
{
}

GraphicsRenderer::~GraphicsRenderer() {
    shutdown();
}

bool GraphicsRenderer::initialize() {
    if (initialized_) {
        return true;
    }

#ifdef HAVE_RAYLIB
    if (config_.isGraphicsEnabled()) {
        // Check if we have a display available before trying to initialize
        const char* display = getenv("DISPLAY");
        if (!display || strlen(display) == 0) {
            std::cerr << "Warning: No DISPLAY environment variable set.\n";
            std::cerr << "Graphics mode is enabled but rendering will be off-screen only.\n";
            initialized_ = false;
            return false;
        }
        
        // Apple II hires resolution is 280x192
        int baseWidth = 280;
        int baseHeight = 192;
        
        windowWidth_ = baseWidth * config_.scaleFactor;
        windowHeight_ = baseHeight * config_.scaleFactor;
        
        // Try to initialize window
        InitWindow(windowWidth_, windowHeight_, "MSBasic - Apple II Graphics");
        
        // Check if window was successfully created
        if (!IsWindowReady()) {
            std::cerr << "Warning: Could not initialize graphics window.\n";
            std::cerr << "Graphics mode is enabled but rendering will be off-screen only.\n";
            CloseWindow();
            initialized_ = false;
            return false;
        }
        
        SetTargetFPS(60);
        
        // Load the Apple II font
        loadApple2Font();
        
        initialized_ = true;
        std::cout << "Graphics window initialized: " << windowWidth_ 
                  << "x" << windowHeight_ << " (scale " 
                  << config_.scaleFactor << ")\n";
        return true;
    }
#endif
    
    // No-graphics mode or Raylib not available
    initialized_ = false;
    return false;
}

void GraphicsRenderer::shutdown() {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        // Font loading not yet implemented, so no cleanup needed
        // TODO: When font is loaded, call UnloadFont() here
        CloseWindow();
        initialized_ = false;
    }
#endif
}

bool GraphicsRenderer::shouldClose() const {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        return WindowShouldClose();
    }
#endif
    return false;
}

void GraphicsRenderer::beginFrame() {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        BeginDrawing();
    }
#endif
}

void GraphicsRenderer::endFrame() {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        EndDrawing();
    }
#endif
}

void GraphicsRenderer::clear() {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        // Apple II background is typically black
        ClearBackground(BLACK);
    }
#endif
}

void GraphicsRenderer::drawPixel(int x, int y, int color) {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        // Scale coordinates
        int scaledX = x * config_.scaleFactor;
        int scaledY = y * config_.scaleFactor;
        
        // Draw a rectangle to represent the pixel at the scaled size
        Color c = {
            static_cast<unsigned char>((color >> 16) & 0xFF),
            static_cast<unsigned char>((color >> 8) & 0xFF),
            static_cast<unsigned char>(color & 0xFF),
            255
        };
        
        DrawRectangle(scaledX, scaledY, config_.scaleFactor, config_.scaleFactor, c);
    }
#endif
}

void GraphicsRenderer::drawText(const std::string& text, int x, int y, int color) {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        // For now, use built-in font
        // TODO: Use Apple II font when loaded
        Color c = {
            static_cast<unsigned char>((color >> 16) & 0xFF),
            static_cast<unsigned char>((color >> 8) & 0xFF),
            static_cast<unsigned char>(color & 0xFF),
            255
        };
        
        DrawText(text.c_str(), x * config_.scaleFactor, y * config_.scaleFactor, 
                 10 * config_.scaleFactor, c);
    }
#endif
}

void GraphicsRenderer::drawChar(char ch, int x, int y, int color) {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        // For now, use built-in drawing
        // TODO: Use Apple II font when loaded
        char str[2] = {ch, '\0'};
        drawText(str, x, y, color);
    }
#endif
}

void GraphicsRenderer::loadApple2Font() {
#ifdef HAVE_RAYLIB
    // Search for the Ultimate Apple II Font in common locations
    const std::vector<const char*> fontPaths = {
        "assets/fonts/PrintChar21.ttf",
        "../assets/fonts/PrintChar21.ttf",
        "/usr/share/fonts/apple2/PrintChar21.ttf"
    };
    
    bool fontFound = false;
    for (const char* path : fontPaths) {
        // Check if file exists - using access() for safer checking
#ifdef _WIN32
        if (_access(path, 0) == 0) {
#else
        if (access(path, F_OK) == 0) {
#endif
            // TODO: Implement actual font loading
            // font_ = LoadFontEx(path, 8, nullptr, 0);
            // fontFound = IsReady(font_);
            std::cout << "Found Apple II font at: " << path << "\n";
            std::cout << "Note: Font loading not yet implemented - using default font\n";
            fontFound = true;
            break;
        }
    }
    
    if (!fontFound) {
        std::cout << "Ultimate Apple II Font not found - using Raylib default font\n";
        std::cout << "Download from: https://www.kreativekorp.com/software/fonts/apple2/\n";
        std::cout << "Place in: assets/fonts/PrintChar21.ttf\n";
    }
#endif
}

unsigned int GraphicsRenderer::convertColor(int appleColor) const {
    // Apple II color palette (simplified)
    // This is a basic approximation of Apple II colors
    static const unsigned int colors[] = {
        0x000000, // 0 - Black
        0xDD0033, // 1 - Magenta/Red
        0x0000DD, // 2 - Dark Blue  
        0xDD00DD, // 3 - Purple
        0x00AA00, // 4 - Dark Green
        0x888888, // 5 - Gray
        0x00AAFF, // 6 - Medium Blue
        0x00FFFF, // 7 - Light Blue/Aqua
        0x885500, // 8 - Brown
        0xFF6600, // 9 - Orange
        0xAAAAAA, // 10 - Gray
        0xFF9999, // 11 - Pink
        0x00FF00, // 12 - Green
        0xFFFF00, // 13 - Yellow
        0xAAFF88, // 14 - Aqua
        0xFFFFFF  // 15 - White
    };
    
    int index = appleColor & 0x0F;  // Clamp to 0-15
    return colors[index];
}
