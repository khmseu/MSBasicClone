/**
 * @file graphics.h
 * @brief Apple II graphics subsystem (GR, HGR, plotting, shapes)
 * 
 * Implements Applesoft BASIC graphics modes and operations:
 * - Low-resolution (GR): 40×48 with 16 colors
 * - High-resolution (HGR/HGR2): 280×192 with 6 colors + mixed modes
 * - Plotting operations (PLOT, HPLOT)
 * - Shape tables (DRAW, XDRAW with ROTATE/SCALE)
 * 
 * Graphics pipeline:
 * 1. Commands (HPLOT, DRAW, etc.) update off-screen buffer
 * 2. Buffer accumulates all operations per frame
 * 3. renderFrame() sends buffer to GraphicsRenderer for display
 * 
 * The Graphics class is a singleton managing graphics state including
 * current color, drawing position, shape transformation matrix, and
 * the frame buffer of plot samples.
 * 
 * Shape operations:
 * - Shape tables contain vector drawing commands
 * - ROTATE and SCALE affect subsequent DRAW/XDRAW
 * - XDRAW uses XOR mode for animation
 * 
 * This design separates graphics logic from platform-specific rendering,
 * with GraphicsRenderer handling actual window/pixel operations.
 */

#pragma once

#include <unordered_map>
#include <vector>
#include <memory>
#include <utility>

// Forward declaration
class GraphicsRenderer;
struct GraphicsConfig;

/**
 * @enum GraphicsMode
 * @brief Current graphics mode
 */
enum class GraphicsMode { 
    None,    ///< Text mode
    LowRes,  ///< GR: 40×48 low-resolution graphics
    HighRes  ///< HGR/HGR2: 280×192 high-resolution graphics
};

/**
 * @struct GraphicsWindow
 * @brief Graphics window dimensions and scaling
 * 
 * Describes the logical and physical dimensions of the graphics display.
 */
struct GraphicsWindow {
  int logicalWidth;      ///< Logical width (40 for GR, 280 for HGR)
  int logicalHeight;     ///< Logical height (48 for GR, 192 for HGR)
  int terminalColumns;   ///< Terminal columns available
  int terminalRows;      ///< Terminal rows available
  double scaleX;         ///< Horizontal scaling factor
  double scaleY;         ///< Vertical scaling factor
};

/**
 * @struct PlotSample
 * @brief Single pixel/plot operation record
 * 
 * Records a single plot operation with both logical and scaled coordinates
 * for rendering. The frame buffer accumulates these for batch rendering.
 */
struct PlotSample {
  double logicalX;  ///< Logical X coordinate (graphics mode units)
  double logicalY;  ///< Logical Y coordinate (graphics mode units)
  int scaledX;      ///< Scaled X for display
  int scaledY;      ///< Scaled Y for display
  int color;        ///< Color value
};

/**
 * @class Graphics
 * @brief Singleton managing graphics state and operations
 * 
 * The Graphics class implements Applesoft graphics commands and maintains
 * graphics state. It uses an off-screen buffer (frame) to accumulate plot
 * operations, which are rendered to the window via GraphicsRenderer.
 * 
 * Thread safety: Not thread-safe. Single-threaded use only.
 * 
 * Usage:
 * @code
 * Graphics::instance().initialize(config);
 * Graphics::instance().enterHighRes();
 * Graphics::instance().setColor(3);
 * Graphics::instance().hplot(140, 96);
 * Graphics::instance().renderFrame();
 * @endcode
 */
class Graphics {
public:
  /**
   * @brief Get singleton instance
   * @return Reference to Graphics singleton
   */
  static Graphics &instance();

  // Initialization
  
  /**
   * @brief Initialize graphics subsystem
   * @param config Graphics configuration
   */
  void initialize(const GraphicsConfig& config);
  
  /**
   * @brief Set platform-specific renderer
   * @param renderer Shared pointer to GraphicsRenderer implementation
   */
  void setRenderer(std::shared_ptr<GraphicsRenderer> renderer);
  
  // Rendering
  
  /**
   * @brief Render accumulated frame to window
   * 
   * Sends all plot operations from the frame buffer to the renderer
   * for display. Call this after graphics commands to update display.
   */
  void renderFrame();
  
  /**
   * @brief Check if window close requested
   * @return true if user closed graphics window
   */
  bool shouldClose() const;

  /**
   * @brief Enter low-resolution graphics mode (GR)
   * 
   * Sets 40×48 graphics with 16 colors. Clears screen.
   */
  void enterLowRes();
  
  /**
   * @brief Enter high-resolution graphics mode (HGR)
   * 
   * Sets 280×192 graphics with 6 colors. Clears screen.
   */
  void enterHighRes();
  
  /**
   * @brief Set drawing color
   * @param color Color value (0-15 for GR, 0-7 for HGR)
   */
  void setColor(int color);
  
  /**
   * @brief Get current drawing color
   * @return Current color value
   */
  int color() const { return color_; }

  /**
   * @brief Get current graphics mode
   * @return Current GraphicsMode
   */
  GraphicsMode mode() const;
  
  /**
   * @brief Get graphics window dimensions
   * @return Reference to GraphicsWindow structure
   */
  const GraphicsWindow &window() const;
  
  /**
   * @brief Get frame buffer
   * @return Reference to vector of PlotSamples
   */
  const std::vector<PlotSample> &frame() const;

  /**
   * @brief Clear frame buffer
   * 
   * Removes all accumulated plot operations without rendering.
   */
  void clearFrame();
  void plot(double x, double y);
  void hlin(double x1, double x2, double y);
  void vlin(double y1, double y2, double x);
  void hplot(double x, double y);
  void hplot_to(double x, double y);
  void move(double x, double y);
  void setRotate(int angle);
  void setScale(int scale);
  void draw(int shapeNum, double x = -1, double y = -1);
  void xdraw(int shapeNum, double x = -1, double y = -1);
  int scrn(double x, double y) const;
  void loadShape(int shapeNum,
                 const std::vector<std::pair<double, double>> &points);

private:
  Graphics();
  void configureWindow(int logicalWidth, int logicalHeight);
  void recordPoint(double x, double y);
  void updatePixel(double x, double y, bool xorMode);
  bool queryTerminalSize(int &columns, int &rows) const;
  void defineDefaultShapes();
  std::pair<double, double> applyTransform(double px, double py) const;
  static long long packKey(int xi, int yi) {
    return (static_cast<long long>(xi) << 32) ^
           static_cast<unsigned long long>(yi);
  }

  GraphicsMode mode_;
  GraphicsWindow window_;
  std::vector<PlotSample> frame_;
  bool windowOpen_;
  int color_;
  std::shared_ptr<GraphicsRenderer> renderer_;  // Actual window renderer
  // Shape drawing state
  int rotateAngle_ = 0; // degrees
  int scaleFactor_ = 1; // unit scale multiplier
  double lastX_ = 0.0;
  double lastY_ = 0.0;
  bool lastValid_ = false;
  // Minimal shape table: polyline points relative to origin
  std::unordered_map<int, std::vector<std::pair<double, double>>> shapeTable_;
  // Simple pixel buffer keyed by integer logical coordinates
  std::unordered_map<long long, int> pixels_;
};

Graphics &graphics();
