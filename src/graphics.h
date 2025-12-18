#pragma once

#include <unordered_map>
#include <vector>

#include <utility>

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
  void hplot(double x, double y);
  void hplot_to(double x, double y);
  void move(double x, double y);
  void setRotate(int angle);
  void setScale(int scale);
  void draw(int shapeNum, double x = -1, double y = -1);
  void xdraw(int shapeNum, double x = -1, double y = -1);

private:
  Graphics();
  void configureWindow(int logicalWidth, int logicalHeight);
  void recordPoint(double x, double y);
  bool queryTerminalSize(int &columns, int &rows) const;
  void defineDefaultShapes();
  std::pair<double, double> applyTransform(double px, double py) const;

  GraphicsMode mode_;
  GraphicsWindow window_;
  std::vector<PlotSample> frame_;
  bool windowOpen_;
  int color_;
  // Shape drawing state
  int rotateAngle_ = 0; // degrees
  int scaleFactor_ = 1; // unit scale multiplier
  double lastX_ = 0.0;
  double lastY_ = 0.0;
  bool lastValid_ = false;
  // Minimal shape table: polyline points relative to origin
  std::unordered_map<int, std::vector<std::pair<double, double>>> shapeTable_;
};

Graphics &graphics();
