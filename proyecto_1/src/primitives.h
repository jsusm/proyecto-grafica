#include "engine2D.h"
#include <functional>
enum class FigureType { Line, unknow };

typedef struct {
  int x;
  int y;
} Point;

class Figure {
public:
  FigureType type = FigureType::unknow;
  bool selected = false;
  virtual void draw(std::function<void(int, int, const Color &)> putPixel) {};
  virtual void select() {}
  virtual void unselect() {}
  virtual void onMouseMove(int, int) { }
  virtual void onMouseButtonDown(int, int) { }
  virtual void onMouseButtonUp(int, int) { }
};

void deployLine(Point start, Point end, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deploySquare(Point vrtx1, Point vrtx2, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deployElipce(Point vrtx1, Point vrtx2, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deployCircle(Point center, int radious, const Color & color, std::function<void(int, int, const Color &)> putPixel);
