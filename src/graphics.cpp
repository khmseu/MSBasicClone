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
      windowOpen_(false), color_(0) {}

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

void Graphics::plot(double x, double y) { recordPoint(x, y); }

void Graphics::hlin(double x1, double x2, double y) {
  recordPoint(x1, y);
  recordPoint(x2, y);
}

void Graphics::vlin(double y1, double y2, double x) {
  recordPoint(x, y1);
  recordPoint(x, y2);
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
