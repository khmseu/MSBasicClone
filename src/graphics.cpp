#include "graphics.h"

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

int clampToInt(double value) { return static_cast<int>(std::lround(value)); }
} // namespace

Graphics::Graphics()
    : mode_(GraphicsMode::None),
      window_{0, 0, kDefaultColumns, kDefaultRows, 1.0, 1.0}, frame_(),
      windowOpen_(false), color_(0) {
  defineDefaultShapes();
}

Graphics &Graphics::instance() {
  static Graphics g;
  return g;
}

Graphics &graphics() { return Graphics::instance(); }

GraphicsMode Graphics::mode() const { return mode_; }

const GraphicsWindow &Graphics::window() const { return window_; }

const std::vector<PlotSample> &Graphics::frame() const { return frame_; }

void Graphics::enterLowRes() {
  mode_ = GraphicsMode::LowRes;
  configureWindow(40, 40);
  clearFrame();
}

void Graphics::enterHighRes() {
  mode_ = GraphicsMode::HighRes;
  configureWindow(280, 192);
  clearFrame();
}

void Graphics::setColor(int color) {
  // Clamp to a byte range; Applesoft color tables used small ints.
  color_ = std::clamp(color, 0, 255);
}

void Graphics::clearFrame() { frame_.clear(); }

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

void Graphics::xdraw(int shapeNum, double x, double y) { draw(shapeNum, x, y); }

void Graphics::defineDefaultShapes() {
  // Basic default shapes for testing: triangle and square, relative coordinates
  shapeTable_[1] = {{0, 0}, {10, 0}, {5, 8}, {0, 0}};
  shapeTable_[2] = {{0, 0}, {10, 0}, {10, 10}, {0, 10}, {0, 0}};
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
