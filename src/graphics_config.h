#pragma once

enum class RenderMode {
    NoGraphics,  // Terminal-only mode, graphics commands cause runtime error
    Graphics     // Full graphics mode with windowed display
};

enum class TextMode {
    Text40,      // 40×24 text mode (default)
    Text80       // 80×24 text mode (via PR#3)
};

struct GraphicsConfig {
    RenderMode mode = RenderMode::Graphics;  // Default to graphics enabled
    int scaleFactor = 2;  // Window scaling multiplier (default 2x)
    TextMode textMode = TextMode::Text40;  // Default to 40-column mode
    
    bool isGraphicsEnabled() const {
        return mode == RenderMode::Graphics;
    }
};
