#pragma once

enum class RenderMode {
    NoGraphics,  // Terminal-only mode, graphics commands cause runtime error
    Graphics     // Full graphics mode with windowed display
};

struct GraphicsConfig {
    RenderMode mode = RenderMode::Graphics;  // Default to graphics enabled
    int scaleFactor = 2;  // Window scaling multiplier (default 2x)
    
    bool isGraphicsEnabled() const {
        return mode == RenderMode::Graphics;
    }
};
