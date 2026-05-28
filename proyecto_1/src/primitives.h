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
  bool mouseOver = false;
  virtual void draw(std::function<void(int, int, const Color &)> putPixel) {};
  virtual void select() {}
  virtual void unselect() {}

  // if this methods return true that means the mouse is "taken"
  // no other figure can interact with the mouse, like when the
  // user is dragging the figure, if return false the mouse
  // can be interacting with other figures
  virtual bool onMouseMove(int, int) { return false; }
  virtual bool onMouseButtonDown(int, int) { return false; }
  virtual bool onMouseButtonUp(int, int) { return false; }
};

void deployLine(Point start, Point end, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deploySquare(Point vrtx1, Point vrtx2, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deployElipce(Point vrtx1, Point vrtx2, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deployCircle(Point center, int radious, const Color & color, std::function<void(int, int, const Color &)> putPixel);
