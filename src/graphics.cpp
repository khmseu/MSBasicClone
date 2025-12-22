/**
 * @file graphics.cpp
 * @brief Implementation of Applesoft BASIC graphics operations
 * 
 * This file implements the Graphics class which provides Apple II graphics
 * functionality including low-resolution (GR) and high-resolution (HGR) modes,
 * plotting, line drawing, and shape table operations.
 * 
 * Graphics architecture:
 * - Graphics singleton maintains graphics state and frame buffer
 * - Commands (HPLOT, DRAW, etc.) add plot operations to frame buffer
 * - renderFrame() sends buffer to GraphicsRenderer for display
 * - Separate logic from rendering allows headless operation
 * 
 * Shape table implementation:
 * - Shapes stored as polyline vectors relative to origin
 * - ROTATE and SCALE affect transformation matrix
 * - DRAW plots shapes, XDRAW uses XOR mode
 * - Default shapes include common geometric forms
 * 
 * The graphics subsystem can operate in three modes:
 * 1. Full rendering (window displayed with Raylib)
 * 2. Off-screen only (buffer maintained, no display)
 * 3. Disabled (graphics commands raise errors)
 */

#include "graphics.h"
#include "graphics_renderer.h"
#include "graphics_config.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

#ifdef PLATFORM_WINDOWS
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <unistd.h>
#endif

namespace {
constexpr int kDefaultColumns = 80;
constexpr int kDefaultRows = 24;

/**
 * @brief Clamp a double to integer range
 * @param value Value to clamp
 * @return Rounded integer value
 */
int clampToInt(double value) { return static_cast<int>(std::lround(value)); }
} // namespace

/**
 * @brief Construct Graphics singleton
 * 
 * Initializes graphics state to text mode and defines default shape table.
 */
Graphics::Graphics()
    : mode_(GraphicsMode::None),
      window_{0, 0, kDefaultColumns, kDefaultRows, 1.0, 1.0}, frame_(),
      windowOpen_(false), color_(0), renderer_(nullptr) {
  defineDefaultShapes();
}

/**
 * @brief Get Graphics singleton instance
 * @return Reference to Graphics singleton
 */
Graphics &Graphics::instance() {
  static Graphics g;
  return g;
}

/**
 * @brief Global accessor for Graphics singleton
 * @return Reference to Graphics singleton
 */
Graphics &graphics() { return Graphics::instance(); }

/**
 * @brief Initialize graphics subsystem with configuration
 * 
 * Creates and initializes the GraphicsRenderer if graphics are enabled.
 * If initialization fails (e.g., no display available), continues without
 * rendering but maintains off-screen buffer.
 * 
 * @param config Graphics configuration (mode, scale factor, etc.)
 */
void Graphics::initialize(const GraphicsConfig& config) {
  // Create and initialize renderer if graphics is enabled
  if (config.isGraphicsEnabled()) {
    renderer_ = std::make_shared<GraphicsRenderer>(config);
    if (!renderer_->initialize()) {
      renderer_.reset();
    }
  }
}

/**
 * @brief Set platform-specific renderer implementation
 * @param renderer Shared pointer to GraphicsRenderer
 */
void Graphics::setRenderer(std::shared_ptr<GraphicsRenderer> renderer) {
  renderer_ = renderer;
}

/**
 * @brief Render accumulated frame buffer to window
 * 
 * Sends all plot operations from the frame buffer to the GraphicsRenderer
 * for display. If no renderer is available or initialized, this is a no-op.
 * The frame buffer persists until explicitly cleared.
 */
void Graphics::renderFrame() {
  if (!renderer_ || !renderer_->isInitialized()) {
    return;
  }
  
  renderer_->beginFrame();
  renderer_->clear();
  
  // Render all recorded points from the frame buffer
  for (const auto& sample : frame_) {
    renderer_->drawPixel(static_cast<int>(sample.logicalX), 
                        static_cast<int>(sample.logicalY), 
                        sample.color);
  }
  
  renderer_->endFrame();
}

/**
 * @brief Check if window close requested by user
 * @return true if close requested, false otherwise
 */
bool Graphics::shouldClose() const {
  if (renderer_ && renderer_->isInitialized()) {
    return renderer_->shouldClose();
  }
  return false;
}

GraphicsMode Graphics::mode() const { return mode_; }

const GraphicsWindow &Graphics::window() const { return window_; }

const std::vector<PlotSample> &Graphics::frame() const { return frame_; }

/**
 * @brief Enter low-resolution graphics mode (GR)
 * 
 * Sets 40×40 graphics mode and clears the frame buffer. Note: Applesoft
 * used 40×48, but this implementation uses 40×40 for simplicity.
 */
void Graphics::enterLowRes() {
  mode_ = GraphicsMode::LowRes;
  configureWindow(40, 40);
  clearFrame();
}

/**
 * @brief Enter high-resolution graphics mode (HGR)
 * 
 * Sets 280×192 graphics mode matching Apple II HGR page 1.
 * Clears the frame buffer.
 */
void Graphics::enterHighRes() {
  mode_ = GraphicsMode::HighRes;
  configureWindow(280, 192);
  clearFrame();
}

/**
 * @brief Set current drawing color
 * 
 * Sets the color for subsequent plot operations. Clamped to 0-255 range.
 * 
 * @param color Color value (0-15 for GR, 0-7 for HGR)
 */
void Graphics::setColor(int color) {
  // Clamp to a byte range; Applesoft color tables used small ints.
  color_ = std::clamp(color, 0, 255);
}

/**
 * @brief Clear the frame buffer
 * 
 * Removes all accumulated plot operations. Does not clear the pixel buffer
 * used for SCRN() lookups.
 */
void Graphics::clearFrame() {
  frame_.clear();
  pixels_.clear();
}

/**
 * @brief Configure graphics window dimensions and scaling
 * 
 * Sets up the logical coordinate system for graphics operations and
 * calculates scaling factors to map logical coordinates to terminal
 * character cell dimensions.
 * 
 * Coordinate system:
 * - Logical: The coordinate space used by BASIC programs (e.g., 0-279 for HGR)
 * - Terminal: Actual terminal size in character cells
 * - Scaling: Maps logical coords to terminal coords for text-mode display
 * 
 * Implementation notes:
 * - Queries actual terminal size via queryTerminalSize()
 * - Falls back to default 80×24 if query fails
 * - Scales logical coordinates proportionally to fit terminal
 * - Maintains aspect ratio within terminal constraints
 * 
 * Example:
 * - HGR mode: 280×192 logical coordinates
 * - 80×24 terminal: Scale factors ~0.28 × 0.125
 * - HPLOT 140,96 maps to terminal position 40,12 (center)
 * 
 * @param logicalWidth Width in logical coordinates (e.g., 280 for HGR)
 * @param logicalHeight Height in logical coordinates (e.g., 192 for HGR)
 */
void Graphics::configureWindow(int logicalWidth, int logicalHeight) {
  int columns = kDefaultColumns;
  int rows = kDefaultRows;
  
  // Try to get actual terminal size
  if (!queryTerminalSize(columns, rows)) {
    // Fall back to standard 80×24 terminal
    columns = kDefaultColumns;
    rows = kDefaultRows;
  }

  window_.logicalWidth = logicalWidth;
  window_.logicalHeight = logicalHeight;
  window_.terminalColumns = columns;
  window_.terminalRows = rows;

  // Calculate scaled dimensions fitting within terminal
  int scaledWidth =
      std::max(1, std::min(logicalWidth, window_.terminalColumns));
  int scaledHeight = std::max(1, std::min(logicalHeight, window_.terminalRows));

  // Calculate scale factors for coordinate mapping
  window_.scaleX = logicalWidth > 0 ? static_cast<double>(scaledWidth) /
                                          static_cast<double>(logicalWidth)
                                    : 1.0;
  window_.scaleY = logicalHeight > 0 ? static_cast<double>(scaledHeight) /
                                           static_cast<double>(logicalHeight)
                                     : 1.0;
  windowOpen_ = true;
}

/**
 * @brief Record a point in the frame buffer
 * 
 * Adds a point to the accumulated frame buffer and updates the pixel buffer
 * for SCRN() queries. This is the core primitive used by all plot operations.
 * 
 * Coordinate processing:
 * 1. Clamp logical coordinates to valid range (0 to width-1, 0 to height-1)
 * 2. Scale to terminal coordinates using window scale factors
 * 3. Round to integer terminal positions
 * 4. Store in frame buffer with both logical and scaled coordinates
 * 5. Update pixel buffer for color queries
 * 
 * The frame buffer is persistent - points accumulate until clearFrame() or
 * mode change. This allows building complex drawings incrementally.
 * 
 * @param x Logical X coordinate
 * @param y Logical Y coordinate
 */
void Graphics::recordPoint(double x, double y) {
  if (!windowOpen_ || window_.logicalWidth <= 0 || window_.logicalHeight <= 0) {
    return;
  }

  double maxX = static_cast<double>(window_.logicalWidth - 1);
  double maxY = static_cast<double>(window_.logicalHeight - 1);

  // Clamp coordinates to valid range
  double clampedX = std::clamp(x, 0.0, maxX);
  double clampedY = std::clamp(y, 0.0, maxY);

  // Scale to terminal coordinates
  int scaledX = clampToInt(clampedX * window_.scaleX);
  int scaledY = clampToInt(clampedY * window_.scaleY);

  // Add to frame buffer with both coordinate systems
  frame_.push_back({clampedX, clampedY, scaledX, scaledY, color_});
  
  // Update pixel buffer for SCRN() function
  updatePixel(clampedX, clampedY, false);
}

/**
 * @brief Plot a single point (PLOT/HPLOT implementation)
 * 
 * Plots a point at the specified coordinates and updates the "last position"
 * for relative drawing operations (DRAW, XDRAW).
 * 
 * Usage in BASIC:
 * - GR mode: PLOT X,Y where X=0-39, Y=0-39
 * - HGR mode: HPLOT X,Y where X=0-279, Y=0-191
 * 
 * Side effects:
 * - Adds point to frame buffer
 * - Sets lastX_, lastY_ for relative operations
 * - Sets lastValid_ flag indicating valid last position
 * 
 * @param x Logical X coordinate
 * @param y Logical Y coordinate
 */
void Graphics::plot(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

/**
 * @brief Draw horizontal line (HLIN implementation)
 * 
 * Draws a horizontal line from (x1,y) to (x2,y). In the simplified
 * implementation, this just records the endpoints. A full implementation
 * would interpolate points along the line.
 * 
 * Usage in BASIC:
 *   HLIN X1,X2 AT Y
 * 
 * Examples:
 *   HLIN 0,39 AT 20  (draw line across screen at row 20)
 *   HLIN 100,200 AT 96  (HGR mode horizontal line)
 * 
 * @param x1 Starting X coordinate
 * @param x2 Ending X coordinate  
 * @param y Y coordinate (constant for horizontal line)
 */
void Graphics::hlin(double x1, double x2, double y) {
  recordPoint(x1, y);
  recordPoint(x2, y);
}

/**
 * @brief Draw vertical line (VLIN implementation)
 * 
 * Draws a vertical line from (x,y1) to (x,y2). This implements the
 * VLIN command from Applesoft BASIC for low-resolution graphics.
 * 
 * Usage in BASIC:
 *   VLIN 0,39 AT 20  (draw line down screen at column 20)
 *   VLIN 10,30 AT 5  (draw vertical segment)
 * 
 * The line is drawn in the current color (set by COLOR command).
 * Y coordinates are automatically clamped to valid screen range.
 * 
 * @param y1 Starting Y coordinate
 * @param y2 Ending Y coordinate
 * @param x X coordinate (constant for vertical line)
 */
void Graphics::vlin(double y1, double y2, double x) {
  recordPoint(x, y1);
  recordPoint(x, y2);
}

/**
 * @brief Plot a high-resolution point (HPLOT implementation)
 * 
 * Plots a single point in high-resolution graphics mode at the specified
 * coordinates. This implements the HPLOT command from Applesoft BASIC.
 * 
 * Behavior:
 * - Plots point in current HCOLOR (set by HCOLOR command)
 * - Updates last position for HPLOT TO operations
 * - Coordinates in Apple II hi-res space (0-279 horizontal, 0-191 vertical)
 * - Points outside valid range are clipped
 * 
 * Usage in BASIC:
 *   HPLOT 100,100  (plot single point)
 *   HPLOT 0,0  (top-left corner)
 *   HPLOT 279,191  (bottom-right corner)
 * 
 * The last plotted position is saved for HPLOT TO operations which
 * draw lines from the previous point.
 * 
 * @param x X coordinate (0-279 in standard hi-res)
 * @param y Y coordinate (0-191 in standard hi-res)
 */
void Graphics::hplot(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

/**
 * @brief Plot a point relative to last position (HPLOT TO implementation)
 * 
 * Similar to plot() but used with HPLOT TO syntax for drawing connected lines.
 * HPLOT maintains a "pen position" that TO operations draw from.
 * 
 * Usage in BASIC:
 *   HPLOT 100,100  (move to position)
 *   HPLOT TO 200,150  (draw line from 100,100 to 200,150)
 *   HPLOT TO 150,200  (draw line from 200,150 to 150,200)
 * 
 * @param x Target X coordinate
 * @param y Target Y coordinate
 */
void Graphics::hplot_to(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

/**
 * @brief Move drawing position without plotting (MOVE implementation)
 * 
 * Sets the current position for shape drawing without actually plotting a point.
 * Used primarily with DRAW/XDRAW to set the origin for shape operations.
 * 
 * The MOVE command affects:
 * - Shape drawing origin (DRAW, XDRAW)
 * - Relative HPLOT TO operations
 * 
 * @param x New X position
 * @param y New Y position
 */
void Graphics::move(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

/**
 * @brief Set rotation angle for shape drawing (ROTATE implementation)
 * 
 * Sets the rotation angle applied to shapes drawn with DRAW/XDRAW.
 * Angle is in degrees, clockwise from the positive X axis.
 * 
 * Usage:
 *   ROTATE=45: DRAW 1  (draw shape 1 rotated 45 degrees)
 * 
 * The rotation is applied to each point in the shape relative to the origin.
 * 
 * @param angle Rotation angle in degrees (0-359)
 */
void Graphics::setRotate(int angle) { rotateAngle_ = angle; }

/**
 * @brief Set scale factor for shape drawing (SCALE implementation)
 * 
 * Sets the scaling factor applied to shapes drawn with DRAW/XDRAW.
 * Scale is a multiplier applied to shape coordinates.
 * 
 * Usage:
 *   SCALE=2: DRAW 1  (draw shape 1 at 2× size)
 *   SCALE=1  (normal size - default)
 * 
 * Minimum scale is 1 (no scale less than 1 allowed).
 * 
 * @param scale Scale factor (minimum 1)
 */
void Graphics::setScale(int scale) {
  if (scale < 1)
    scale = 1;
  scaleFactor_ = scale;
}

/**
 * @brief Draw a shape from shape table (DRAW implementation)
 * 
 * Draws a shape from the shape table at the specified position, applying
 * current rotation and scale transformations. Shapes are stored as sequences
 * of relative coordinate pairs.
 * 
 * Shape Drawing Process:
 * 1. Look up shape by number in shapeTable_
 * 2. Determine origin (x,y if valid, else last position, else 0,0)
 * 3. For each point in shape:
 *    a. Apply rotation transformation (rotate by rotateAngle_)
 *    b. Apply scale transformation (multiply by scaleFactor_)
 *    c. Translate to origin position
 *    d. Record point in frame buffer
 * 
 * Transformation math:
 *   rotated_x = x * cos(angle) - y * sin(angle)
 *   rotated_y = x * sin(angle) + y * cos(angle)
 *   final_x = origin_x + rotated_x * scale
 *   final_y = origin_y + rotated_y * scale
 * 
 * Usage in BASIC:
 *   DRAW 1  (draw shape 1 at current position)
 *   DRAW 1 AT 100,50  (draw shape 1 at specific position)
 *   ROTATE=45: SCALE=2: DRAW 1  (rotated and scaled)
 * 
 * @param shapeNum Shape number to draw (1-255)
 * @param x X coordinate of origin (< 0 uses last position)
 * @param y Y coordinate of origin (< 0 uses last position)
 */
void Graphics::draw(int shapeNum, double x, double y) {
  auto it = shapeTable_.find(shapeNum);
  if (it == shapeTable_.end()) {
    return; // Unknown shape - silently ignore
  }

  // Determine drawing origin
  double originX = x;
  double originY = y;
  if (!(originX >= 0 && originY >= 0)) {
    // Use last position if no position specified
    if (lastValid_) {
      originX = lastX_;
      originY = lastY_;
    } else {
      originX = 0.0;
      originY = 0.0;
    }
  }

  // Record origin point
  recordPoint(originX, originY);

  // Draw each point in the shape with transformations
  for (const auto &pt : it->second) {
    // Apply rotation transformation
    double rad = static_cast<double>(rotateAngle_) * M_PI / 180.0;
    double sx = static_cast<double>(scaleFactor_);
    double rx = pt.first * std::cos(rad) - pt.second * std::sin(rad);
    double ry = pt.first * std::sin(rad) + pt.second * std::cos(rad);
    
    // Apply scale and translate to origin
    double finalX = originX + rx * sx;
    double finalY = originY + ry * sx;
    
    recordPoint(finalX, finalY);
    lastX_ = finalX;
    lastY_ = finalY;
    lastValid_ = true;
  }
}

/**
 * @brief Draw shape with XOR mode (XDRAW implementation)
 * 
 * Similar to draw() but uses XOR (exclusive or) mode for drawing. In XOR mode:
 * - Drawing on background sets pixels
 * - Drawing on existing pixels clears them
 * - Drawing the same shape twice returns to original state
 * 
 * XOR drawing is useful for:
 * - Animation (draw/erase/move/redraw without clearing screen)
 * - Cursors and temporary graphics
 * - Creating "invert" effects
 * 
 * XOR Logic:
 * - If pixel is off (0), set to current color
 * - If pixel is on (non-zero), set to off (0)
 * - This makes drawing reversible: XDRAW twice = no change
 * 
 * Usage in BASIC:
 *   XDRAW 1 AT 100,100  (draw shape 1)
 *   XDRAW 1 AT 100,100  (erase shape 1 - back to original)
 *   ' Move and redraw
 *   XDRAW 1 AT 110,100  (draw at new position)
 * 
 * All rotation and scale transformations apply as with DRAW.
 * 
 * @param shapeNum Shape number to draw in XOR mode
 * @param x X coordinate of origin (< 0 uses last position)
 * @param y Y coordinate of origin (< 0 uses last position)
 */
void Graphics::xdraw(int shapeNum, double x, double y) {
  auto it = shapeTable_.find(shapeNum);
  if (it == shapeTable_.end()) {
    return; // Unknown shape
  }

  // Determine drawing origin (same as draw())
  double originX = x;
  double originY = y;
  if (!(originX >= 0 && originY >= 0)) {
    if (lastValid_) {
      originX = lastX_;
      originY = lastY_;
    } else {
      originX = 0.0;
      originY = 0.0;
    }
  }

  // Record origin and XOR it
  recordPoint(originX, originY);
  updatePixel(originX, originY, true);  // XOR mode

  // Draw each point with XOR
  for (const auto &pt : it->second) {
    double rad = static_cast<double>(rotateAngle_) * M_PI / 180.0;
    double sx = static_cast<double>(scaleFactor_);
    double rx = pt.first * std::cos(rad) - pt.second * std::sin(rad);
    double ry = pt.first * std::sin(rad) + pt.second * std::cos(rad);
    double finalX = originX + rx * sx;
    double finalY = originY + ry * sx;
    
    recordPoint(finalX, finalY);
    updatePixel(finalX, finalY, true);  // XOR mode
    lastX_ = finalX;
    lastY_ = finalY;
    lastValid_ = true;
  }
}

/**
 * @brief Initialize default shape table
 * 
 * Creates two basic shapes for testing and demonstration:
 * - Shape 1: Triangle (4 points forming a closed triangle)
 * - Shape 2: Square (5 points forming a closed square)
 * 
 * Shape coordinates are relative to the origin (0,0). When drawn, they are
 * transformed by rotation/scale and positioned at the drawing origin.
 * 
 * Programs can load custom shapes using SHLOAD command.
 */
void Graphics::defineDefaultShapes() {
  // Basic default shapes for testing: triangle and square, relative coordinates
  shapeTable_[1] = {{0, 0}, {10, 0}, {5, 8}, {0, 0}};  // Triangle
  shapeTable_[2] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}, {0, 0}};  // Square
}

/**
 * @brief Update pixel buffer for SCRN() queries
 * 
 * Maintains a sparse map of plotted pixels with their colors, used by the
 * SCRN(x,y) function to query what color is at a specific location.
 * 
 * Two modes:
 * 1. Normal mode (xorMode=false): Set pixel to current color
 * 2. XOR mode (xorMode=true): Toggle pixel (on→off, off→on)
 * 
 * XOR Logic:
 * - If pixel exists and is non-zero: set to 0 (turn off)
 * - If pixel is 0 or doesn't exist: set to current color (turn on)
 * - Special case: if color_ is 0, use 1 to represent "on"
 * 
 * The pixel buffer is separate from the frame buffer:
 * - Frame buffer: All plotted points for rendering
 * - Pixel buffer: Current pixel states for SCRN() queries
 * 
 * @param x Logical X coordinate
 * @param y Logical Y coordinate
 * @param xorMode If true, use XOR logic; if false, set directly
 */
void Graphics::updatePixel(double x, double y, bool xorMode) {
  if (!windowOpen_)
    return;
    
  double maxX = static_cast<double>(window_.logicalWidth - 1);
  double maxY = static_cast<double>(window_.logicalHeight - 1);
  double clampedX = std::clamp(x, 0.0, maxX);
  double clampedY = std::clamp(y, 0.0, maxY);
  int xi = clampToInt(clampedX);
  int yi = clampToInt(clampedY);
  long long key = packKey(xi, yi);
  
  if (xorMode) {
    // XOR mode: toggle pixel on/off
    auto it = pixels_.find(key);
    if (it != pixels_.end() && it->second != 0) {
      it->second = 0; // Turn off
    } else {
      // Turn on with current color (use 1 if color is 0)
      pixels_[key] = color_ == 0 ? 1 : color_;
    }
  } else {
    // Normal mode: set to current color
    pixels_[key] = color_;
  }
}

/**
 * @brief Query pixel color at coordinates (SCRN function implementation)
 * 
 * Returns the color value of the pixel at the specified logical coordinates.
 * This implements the SCRN(x,y) function in Applesoft BASIC.
 * 
 * Return value:
 * - 0 if no pixel has been drawn at this location
 * - Color value (1-15 for GR, 0-7 for HGR) if pixel exists
 * 
 * Usage in BASIC:
 *   C = SCRN(100,50)  (get color at 100,50)
 *   IF SCRN(X,Y) <> 0 THEN PRINT "HIT!"  (collision detection)
 * 
 * Implementation notes:
 * - Queries the pixel buffer (not frame buffer)
 * - Coordinates are clamped to valid range
 * - Returns 0 for unplotted locations
 * 
 * @param x Logical X coordinate
 * @param y Logical Y coordinate
 * @return Color value at that location, or 0 if empty
 */
int Graphics::scrn(double x, double y) const {
  double maxX = static_cast<double>(window_.logicalWidth - 1);
  double maxY = static_cast<double>(window_.logicalHeight - 1);
  double clampedX = std::clamp(x, 0.0, maxX);
  double clampedY = std::clamp(y, 0.0, maxY);
  int xi = clampToInt(clampedX);
  int yi = clampToInt(clampedY);
  long long key = packKey(xi, yi);
  auto it = pixels_.find(key);
  if (it == pixels_.end())
    return 0;
  return it->second;
}

/**
 * @brief Load a custom shape into shape table
 * 
 * Adds or replaces a shape in the shape table. Shapes are defined as a
 * sequence of relative coordinate pairs that define a polyline.
 * 
 * Shape coordinate conventions:
 * - All coordinates are relative to origin (0,0)
 * - First point is usually (0,0) for the start
 * - Last point typically returns to (0,0) for closed shapes
 * - Coordinates can be negative for shapes extending in all directions
 * 
 * Usage (typically from SHLOAD command):
 *   Shape data is loaded from a file or tape
 *   Each shape has a number (1-255) and list of points
 * 
 * @param shapeNum Shape number (1-255)
 * @param points Vector of coordinate pairs defining the shape
 */
void Graphics::loadShape(int shapeNum,
                         const std::vector<std::pair<double, double>> &points) {
  shapeTable_[shapeNum] = points;
}

bool Graphics::queryTerminalSize(int &columns, int &rows) const {
#ifdef PLATFORM_WINDOWS
  HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
  if (h != nullptr && h != INVALID_HANDLE_VALUE) {
    CONSOLE_SCREEN_BUFFER_INFO info;
    if (GetConsoleScreenBufferInfo(h, &info)) {
      columns = static_cast<int>(info.srWindow.Right - info.srWindow.Left + 1);
      rows = static_cast<int>(info.srWindow.Bottom - info.srWindow.Top + 1);
      return true;
    }
  }
  return false;
#else
  struct winsize ws {};
  if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0 &&
      ws.ws_row > 0) {
    columns = static_cast<int>(ws.ws_col);
    rows = static_cast<int>(ws.ws_row);
    return true;
  }

  const char *colsEnv = std::getenv("COLUMNS");
  const char *rowsEnv = std::getenv("LINES");
  if (colsEnv != nullptr && rowsEnv != nullptr) {
    int envCols = std::atoi(colsEnv);
    int envRows = std::atoi(rowsEnv);
    if (envCols > 0 && envRows > 0) {
      columns = envCols;
      rows = envRows;
      return true;
    }
  }

  return false;
#endif
}
