/**
 * @file graphics_renderer.h
 * @brief Platform-specific graphics rendering abstraction
 * 
 * The GraphicsRenderer class provides a platform-independent interface for
 * rendering graphics operations. It handles window creation, pixel drawing,
 * text rendering, and frame management using the Raylib graphics library.
 * 
 * Responsibilities:
 * - Window initialization and lifecycle management
 * - Pixel-level drawing operations
 * - Text and character rendering with Apple II font
 * - Color conversion from Apple II palette to RGB
 * - Frame timing and display updates
 * 
 * The renderer is designed to be swappable, allowing different rendering
 * backends (terminal, SDL, OpenGL, etc.) by implementing this interface.
 * 
 * Compilation:
 * - Requires HAVE_RAYLIB to be defined for Raylib support
 * - Falls back to stub implementation if graphics library unavailable
 * 
 * Font:
 * - Uses authentic Apple II font bitmap for text rendering
 * - Font loaded from assets/fonts/apple2.ttf at initialization
 * - Fallback to default font if Apple II font unavailable
 */

#pragma once

#include "graphics_config.h"
#include <string>
#include <vector>

// Forward declarations for Raylib types (to avoid including raylib.h in header)
struct Color;
struct Font;

/**
 * @class GraphicsRenderer
 * @brief Platform-specific graphics rendering implementation
 * 
 * Implements window management and rendering operations using Raylib.
 * Provides pixel drawing, text rendering with Apple II font, and frame
 * management for the Graphics subsystem.
 * 
 * Lifecycle:
 * 1. Construct with GraphicsConfig
 * 2. Call initialize() to create window and load resources
 * 3. Call beginFrame() before drawing
 * 4. Perform drawing operations (drawPixel, drawText, etc.)
 * 5. Call endFrame() to present frame
 * 6. Call shutdown() when done
 * 
 * The renderer maintains window state and handles platform-specific
 * rendering details, isolating the Graphics class from platform APIs.
 */
class GraphicsRenderer {
public:
    /**
     * @brief Construct renderer with configuration
     * @param config Graphics configuration (mode, scale, text mode)
     */
    GraphicsRenderer(const GraphicsConfig& config);
    
    /**
     * @brief Destructor, calls shutdown if needed
     */
    ~GraphicsRenderer();

    // Window management
    
    /**
     * @brief Initialize window and resources
     * @return true if initialization successful
     * 
     * Creates window, loads Apple II font, sets up rendering context.
     * Must be called before any drawing operations.
     */
    bool initialize();
    
    /**
     * @brief Shutdown renderer and close window
     * 
     * Releases resources and closes window. Safe to call multiple times.
     */
    void shutdown();
    
    /**
     * @brief Check if renderer is initialized
     * @return true if initialize() succeeded and window is open
     */
    bool isInitialized() const { return initialized_; }
    
    /**
     * @brief Check if user requested window close
     * @return true if window close button clicked
     */
    bool shouldClose() const;

    // Rendering
    
    /**
     * @brief Begin rendering frame
     * 
     * Call before any drawing operations. Prepares rendering context.
     */
    void beginFrame();
    
    /**
     * @brief End rendering frame and present to screen
     * 
     * Finalizes frame and swaps buffers. Call after all drawing operations.
     */
    void endFrame();
    
    /**
     * @brief Clear screen to black
     * 
     * Fills entire window with black. Typically called at start of frame.
     */
    void clear();
    
    // Pixel operations (for HPLOT, PLOT, etc.)
    
    /**
     * @brief Draw single pixel
     * @param x X coordinate (window coordinates)
     * @param y Y coordinate (window coordinates)
     * @param color Apple II color value
     * 
     * Draws a pixel in Apple II color at the specified position.
     */
    void drawPixel(int x, int y, int color);
    
    // Text rendering
    
    /**
     * @brief Draw text string
     * @param text String to render
     * @param x X position (pixels)
     * @param y Y position (pixels)
     * @param color Apple II color value
     * 
     * Renders text using Apple II font at specified position.
     */
    void drawText(const std::string& text, int x, int y, int color);
    
    /**
     * @brief Draw single character
     * @param ch Character to render
     * @param x X position (pixels)
     * @param y Y position (pixels)
     * @param color Apple II color value
     * 
     * Renders single character using Apple II font.
     */
    void drawChar(char ch, int x, int y, int color);
    
    // Mode information
    
    /**
     * @brief Get window width
     * @return Window width in pixels
     */
    int getWindowWidth() const { return windowWidth_; }
    
    /**
     * @brief Get window height
     * @return Window height in pixels
     */
    int getWindowHeight() const { return windowHeight_; }
    
    /**
     * @brief Get scale factor from configuration
     * @return Scale multiplier
     */
    int getScaleFactor() const { return config_.scaleFactor; }

private:
    GraphicsConfig config_;  ///< Rendering configuration
    bool initialized_;       ///< Initialization state
    int windowWidth_;        ///< Scaled window width (pixels)
    int windowHeight_;       ///< Scaled window height (pixels)
    
#ifdef HAVE_RAYLIB
    Font font_;              ///< Apple II font (default-constructed if not loaded)
    bool fontLoaded_;        ///< Track if font was successfully loaded
#endif

    /**
     * @brief Load Apple II font from assets
     * 
     * Attempts to load assets/fonts/apple2.ttf. Falls back to default
     * font if unavailable.
     */
    void loadApple2Font();
    
    /**
     * @brief Convert Apple II color to RGB
     * @param appleColor Apple II color value (0-15)
     * @return RGB color value for Raylib
     */
    unsigned int convertColor(int appleColor) const;
};
