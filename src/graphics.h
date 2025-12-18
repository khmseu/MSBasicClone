#pragma once

#include <vector>

enum class GraphicsMode { None, LowRes, HighRes };

struct GraphicsWindow {
  int logicalWidth;
  int logicalHeight;
  int terminalColumns;
  int terminalRows;
  double scaleX;
  double scaleY;
};

struct PlotSample {
  double logicalX;
  double logicalY;
  int scaledX;
  int scaledY;
  int color;
};

class Graphics {
public:
  static Graphics &instance();

  void enterLowRes();
  void enterHighRes();
  void setColor(int color);
  int color() const { return color_; }

  GraphicsMode mode() const;
  const GraphicsWindow &window() const;
  const std::vector<PlotSample> &frame() const;

  void clearFrame();
  void plot(double x, double y);
  void hlin(double x1, double x2, double y);
  void vlin(double y1, double y2, double x);

private:
  Graphics();
  void configureWindow(int logicalWidth, int logicalHeight);
  void recordPoint(double x, double y);
  bool queryTerminalSize(int &columns, int &rows) const;

  GraphicsMode mode_;
  GraphicsWindow window_;
  std::vector<PlotSample> frame_;
  bool windowOpen_;
  int color_;
};

Graphics &graphics();
