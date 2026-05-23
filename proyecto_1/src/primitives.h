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
  virtual bool onMouseMove(int, int) { return false; }
  virtual bool onMouseClick(int, int) { return false; }
  virtual bool isMouseOver(int, int) { return false; }
};

void deployLine(Point start, Point end, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deploySquare(Point vrtx1, Point vrtx2, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deployElipce(Point vrtx1, Point vrtx2, const Color & color, std::function<void(int, int, const Color &)> putPixel);

void deployCircle(Point center, Point radious, const Color & color, std::function<void(int, int, const Color &)> putPixel);
