#include "engine2D.h"
#include <functional>
enum class FigureType { Line, unknow };

class Figure {
public:
  FigureType type = FigureType::unknow;
  bool selected = false;
  virtual void draw(std::function<void(int, int, const Color &)> putPixel) {};
  virtual void select() {}
  virtual void unselect() {}
  virtual bool onMouseMove(int, int) { return false; }
  virtual bool onMouseClick(int, int) { return false; }
};

typedef struct {
  int x;
  int y;
} Point;

void deployLine(Point start, Point end, std::function<void(int, int, const Color &)> putPixel);
