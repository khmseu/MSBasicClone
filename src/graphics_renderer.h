#pragma once

#include "graphics_config.h"
#include <string>
#include <vector>

// Forward declarations for Raylib types (to avoid including raylib.h in header)
struct Color;
struct Font;

// Graphics renderer abstraction - handles actual window rendering
class GraphicsRenderer {
public:
    GraphicsRenderer(const GraphicsConfig& config);
    ~GraphicsRenderer();

    // Window management
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized_; }
    bool shouldClose() const;

    // Rendering
    void beginFrame();
    void endFrame();
    void clear();
    
    // Pixel operations (for HPLOT, PLOT, etc.)
    void drawPixel(int x, int y, int color);
    
    // Text rendering
    void drawText(const std::string& text, int x, int y, int color);
    void drawChar(char ch, int x, int y, int color);
    
    // Mode information
    int getWindowWidth() const { return windowWidth_; }
    int getWindowHeight() const { return windowHeight_; }
    int getScaleFactor() const { return config_.scaleFactor; }

private:
    GraphicsConfig config_;
    bool initialized_;
    int windowWidth_;   // Scaled window width
    int windowHeight_;  // Scaled window height
    
#ifdef HAVE_RAYLIB
    Font* font_;        // Apple II font (pointer to handle optional loading)
    bool fontLoaded_;   // Track if font was successfully loaded
#endif

    void loadApple2Font();
    unsigned int convertColor(int appleColor) const;
};
