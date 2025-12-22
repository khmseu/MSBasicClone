/**
 * @file graphics_renderer.cpp
 * @brief Platform-specific graphics rendering implementation using Raylib
 * 
 * This file implements the GraphicsRenderer class which provides actual
 * window creation and pixel rendering for MSBasic graphics commands.
 * It uses the Raylib library when available for cross-platform graphics.
 * 
 * Key features:
 * - Window creation with configurable scale factor
 * - Pixel-perfect Apple II graphics rendering (280×192 base resolution)
 * - Apple II font loading and rendering for text in graphics mode
 * - Graceful fallback when display not available (headless mode)
 * - Double-buffering via BeginDrawing/EndDrawing
 * 
 * The renderer is separate from Graphics logic to allow:
 * 1. Testing graphics logic without display
 * 2. Supporting multiple rendering backends
 * 3. Running in headless environments (CI/CD, servers)
 * 
 * Conditional compilation:
 * - HAVE_RAYLIB: Enables Raylib-based rendering
 * - Without Raylib: Stubs return false, graphics commands work off-screen only
 */

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

/**
 * @brief Construct a GraphicsRenderer
 * @param config Graphics configuration (scale, mode, etc.)
 */
GraphicsRenderer::GraphicsRenderer(const GraphicsConfig& config)
    : config_(config), initialized_(false), windowWidth_(0), windowHeight_(0)
#ifdef HAVE_RAYLIB
    , font_(), fontLoaded_(false)
#endif
{
}

/**
 * @brief Destructor - ensures proper cleanup
 */
GraphicsRenderer::~GraphicsRenderer() {
    shutdown();
}

/**
 * @brief Initialize the graphics window and renderer
 * 
 * Creates a Raylib window scaled to the configured factor (default 2x).
 * Base resolution is 280×192 matching Apple II high-res mode.
 * 
 * Performs checks:
 * - Verifies DISPLAY environment variable (Unix/Linux)
 * - Tests window creation success
 * - Loads Apple II font if available
 * 
 * Returns false if initialization fails, allowing off-screen operation.
 * 
 * @return true if window created successfully, false otherwise
 */
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

/**
 * @brief Shutdown and cleanup the renderer
 * 
 * Unloads fonts, closes the window, and marks renderer as uninitialized.
 * Safe to call multiple times.
 */
void GraphicsRenderer::shutdown() {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        // Unload font if it was loaded
        if (fontLoaded_) {
            UnloadFont(font_);
            fontLoaded_ = false;
        }
        CloseWindow();
        initialized_ = false;
    }
#endif
}

/**
 * @brief Check if user requested window close
 * @return true if window close requested, false otherwise
 */
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
        Color c = {
            static_cast<unsigned char>((color >> 16) & 0xFF),
            static_cast<unsigned char>((color >> 8) & 0xFF),
            static_cast<unsigned char>(color & 0xFF),
            255
        };
        
        if (fontLoaded_) {
            // Use loaded Apple II font
            // Apple II character cell is 7×8 pixels
            float fontSize = 8.0f * config_.scaleFactor;
            float spacing = 1.0f * config_.scaleFactor;
            
            // Apply horizontal scaling for 80-column mode (0.5x)
            float horizontalScale = 1.0f;
            if (config_.textMode == TextMode::Text80) {
                horizontalScale = 0.5f;
            }
            
            Vector2 position = {
                static_cast<float>(x * config_.scaleFactor * horizontalScale),
                static_cast<float>(y * config_.scaleFactor)
            };
            
            // If we're in 80-column mode, we need to scale the text horizontally
            if (horizontalScale != 1.0f) {
                // Pre-allocate character buffer for efficiency
                char charBuffer[2] = {'\0', '\0'};
                // Draw each character with adjusted spacing for horizontal compression
                float currentX = position.x;
                for (const char& ch : text) {
                    charBuffer[0] = ch;
                    DrawTextEx(font_, charBuffer, (Vector2){currentX, position.y}, fontSize, 0, c);
                    // In 80-column mode, characters are compressed horizontally
                    currentX += (7.0f * config_.scaleFactor * horizontalScale);
                }
            } else {
                // Normal 40-column mode rendering
                DrawTextEx(font_, text.c_str(), position, fontSize, spacing, c);
            }
        } else {
            // Fallback to built-in font
            DrawText(text.c_str(), x * config_.scaleFactor, y * config_.scaleFactor, 
                     10 * config_.scaleFactor, c);
        }
    }
#endif
}

void GraphicsRenderer::drawChar(char ch, int x, int y, int color) {
#ifdef HAVE_RAYLIB
    if (initialized_) {
        Color c = {
            static_cast<unsigned char>((color >> 16) & 0xFF),
            static_cast<unsigned char>((color >> 8) & 0xFF),
            static_cast<unsigned char>(color & 0xFF),
            255
        };
        
        if (fontLoaded_) {
            // Use loaded Apple II font for single character
            // Apple II character cell is 7×8 pixels
            float fontSize = 8.0f * config_.scaleFactor;
            
            // Apply horizontal scaling for 80-column mode (0.5x)
            float horizontalScale = 1.0f;
            if (config_.textMode == TextMode::Text80) {
                horizontalScale = 0.5f;
            }
            
            Vector2 position = {
                static_cast<float>(x * config_.scaleFactor * horizontalScale),
                static_cast<float>(y * config_.scaleFactor)
            };
            
            char str[2] = {ch, '\0'};
            DrawTextEx(font_, str, position, fontSize, 0, c);
        } else {
            // Fallback to built-in drawing
            char str[2] = {ch, '\0'};
            drawText(str, x, y, color);
        }
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
    const char* foundPath = nullptr;
    
    for (const char* path : fontPaths) {
        // Check if file exists - using access() for safer checking
#ifdef _WIN32
        if (_access(path, 0) == 0) {
#else
        if (access(path, F_OK) == 0) {
#endif
            foundPath = path;
            fontFound = true;
            break;
        }
    }
    
    if (fontFound) {
        try {
            // Load font at 8 pixels (Apple II character cell is 7×8)
            // We use 8 here as the fontSize to get proper glyph rendering
            font_ = LoadFontEx(foundPath, 8, nullptr, 0);
            
            // Check if font loaded successfully using IsFontReady()
            if (IsFontReady(font_)) {
                // Apply bilinear filtering for better quality when scaling
                SetTextureFilter(font_.texture, TEXTURE_FILTER_BILINEAR);
                
                fontLoaded_ = true;
                std::cout << "Successfully loaded Apple II font from: " << foundPath << "\n";
                std::cout << "Font base size: " << font_.baseSize << " pixels\n";
            } else {
                std::cerr << "Warning: Font file found but failed to load from: " << foundPath << "\n";
                // Unload the font to ensure proper cleanup of any partially loaded resources
                UnloadFont(font_);
                fontLoaded_ = false;
            }
        } catch (...) {
            std::cerr << "Warning: Exception while loading font from: " << foundPath << "\n";
            // Attempt to unload font in case of partial initialization
            try {
                UnloadFont(font_);
            } catch (...) {
                // Ignore cleanup errors
            }
            fontLoaded_ = false;
        }
    }
    
    if (!fontLoaded_) {
        std::cout << "Ultimate Apple II Font not found or failed to load - using Raylib default font\n";
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
