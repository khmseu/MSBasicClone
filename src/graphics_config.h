/**
 * @file graphics_config.h
 * @brief Graphics and text mode configuration
 * 
 * Defines configuration settings for graphics rendering and text modes.
 * Controls whether graphics features are enabled and how they are rendered.
 * 
 * Render modes:
 * - NoGraphics: Terminal-only mode, graphics commands cause runtime errors
 * - Graphics: Full graphics with windowed display (requires Raylib)
 * 
 * Text modes:
 * - Text40: Standard 40-column by 24-row mode (Applesoft default)
 * - Text80: Extended 80-column by 24-row mode (via PR#3 command)
 * 
 * Scale factor controls the window size multiplier for graphics display,
 * allowing larger windows on modern displays (e.g., 2x = 560×384 for HGR).
 */

#pragma once

/**
 * @enum RenderMode
 * @brief Graphics rendering mode selection
 */
enum class RenderMode {
    NoGraphics,  ///< Terminal-only mode, graphics commands cause runtime error
    Graphics     ///< Full graphics mode with windowed display
};

/**
 * @enum TextMode
 * @brief Text display mode (column width)
 */
enum class TextMode {
    Text40,      ///< 40×24 text mode (Applesoft default)
    Text80       ///< 80×24 text mode (via PR#3 slot redirection)
};

/**
 * @struct GraphicsConfig
 * @brief Configuration for graphics and text rendering
 * 
 * Controls the runtime behavior of graphics operations and text display.
 * Can be configured to disable graphics for terminal-only execution.
 */
struct GraphicsConfig {
    RenderMode mode = RenderMode::Graphics;  ///< Graphics enabled/disabled (default: Graphics)
    int scaleFactor = 2;                     ///< Window scaling multiplier (default: 2x)
    TextMode textMode = TextMode::Text40;    ///< Text width mode (default: 40-column)
    
    /**
     * @brief Check if graphics operations are enabled
     * @return true if graphics mode is active
     */
    bool isGraphicsEnabled() const {
        return mode == RenderMode::Graphics;
    }
};
