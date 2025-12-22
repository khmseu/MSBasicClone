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

void Graphics::configureWindow(int logicalWidth, int logicalHeight) {
  int columns = kDefaultColumns;
  int rows = kDefaultRows;
  if (!queryTerminalSize(columns, rows)) {
    columns = kDefaultColumns;
    rows = kDefaultRows;
  }

  window_.logicalWidth = logicalWidth;
  window_.logicalHeight = logicalHeight;
  window_.terminalColumns = columns;
  window_.terminalRows = rows;

  int scaledWidth =
      std::max(1, std::min(logicalWidth, window_.terminalColumns));
  int scaledHeight = std::max(1, std::min(logicalHeight, window_.terminalRows));

  window_.scaleX = logicalWidth > 0 ? static_cast<double>(scaledWidth) /
                                          static_cast<double>(logicalWidth)
                                    : 1.0;
  window_.scaleY = logicalHeight > 0 ? static_cast<double>(scaledHeight) /
                                           static_cast<double>(logicalHeight)
                                     : 1.0;
  windowOpen_ = true;
}

void Graphics::recordPoint(double x, double y) {
  if (!windowOpen_ || window_.logicalWidth <= 0 || window_.logicalHeight <= 0) {
    return;
  }

  double maxX = static_cast<double>(window_.logicalWidth - 1);
  double maxY = static_cast<double>(window_.logicalHeight - 1);

  double clampedX = std::clamp(x, 0.0, maxX);
  double clampedY = std::clamp(y, 0.0, maxY);

  int scaledX = clampToInt(clampedX * window_.scaleX);
  int scaledY = clampToInt(clampedY * window_.scaleY);

  frame_.push_back({clampedX, clampedY, scaledX, scaledY, color_});
  updatePixel(clampedX, clampedY, false);
}

void Graphics::plot(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

void Graphics::hlin(double x1, double x2, double y) {
  recordPoint(x1, y);
  recordPoint(x2, y);
}

void Graphics::vlin(double y1, double y2, double x) {
  recordPoint(x, y1);
  recordPoint(x, y2);
}

void Graphics::hplot(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

void Graphics::hplot_to(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

void Graphics::move(double x, double y) {
  recordPoint(x, y);
  lastX_ = x;
  lastY_ = y;
  lastValid_ = true;
}

void Graphics::setRotate(int angle) { rotateAngle_ = angle; }

void Graphics::setScale(int scale) {
  if (scale < 1)
    scale = 1;
  scaleFactor_ = scale;
}

void Graphics::draw(int shapeNum, double x, double y) {
  auto it = shapeTable_.find(shapeNum);
  if (it == shapeTable_.end()) {
    return; // unknown shape
  }

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

  // Record origin for context
  recordPoint(originX, originY);

  for (const auto &pt : it->second) {
    // Apply rotation + scale to relative point
    double rad = static_cast<double>(rotateAngle_) * M_PI / 180.0;
    double sx = static_cast<double>(scaleFactor_);
    double rx = pt.first * std::cos(rad) - pt.second * std::sin(rad);
    double ry = pt.first * std::sin(rad) + pt.second * std::cos(rad);
    double finalX = originX + rx * sx;
    double finalY = originY + ry * sx;
    recordPoint(finalX, finalY);
    lastX_ = finalX;
    lastY_ = finalY;
    lastValid_ = true;
  }
}

void Graphics::xdraw(int shapeNum, double x, double y) {
  auto it = shapeTable_.find(shapeNum);
  if (it == shapeTable_.end()) {
    return; // unknown shape
  }

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

  // Record origin for context and XOR it
  recordPoint(originX, originY);
  updatePixel(originX, originY, true);

  for (const auto &pt : it->second) {
    double rad = static_cast<double>(rotateAngle_) * M_PI / 180.0;
    double sx = static_cast<double>(scaleFactor_);
    double rx = pt.first * std::cos(rad) - pt.second * std::sin(rad);
    double ry = pt.first * std::sin(rad) + pt.second * std::cos(rad);
    double finalX = originX + rx * sx;
    double finalY = originY + ry * sx;
    recordPoint(finalX, finalY);
    updatePixel(finalX, finalY, true);
    lastX_ = finalX;
    lastY_ = finalY;
    lastValid_ = true;
  }
}

void Graphics::defineDefaultShapes() {
  // Basic default shapes for testing: triangle and square, relative coordinates
  shapeTable_[1] = {{0, 0}, {10, 0}, {5, 8}, {0, 0}};
  shapeTable_[2] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}, {0, 0}};
}

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
    auto it = pixels_.find(key);
    if (it != pixels_.end() && it->second != 0) {
      it->second = 0; // toggle off
    } else {
      pixels_[key] =
          color_ == 0
              ? 1
              : color_; // toggle on with current color (use 1 if color is 0)
    }
  } else {
    pixels_[key] = color_;
  }
}

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
