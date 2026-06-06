#include "primitives.h"
#include <algorithm>
#include <cmath>
#include <functional>

void deployLine(Point start, Point end, const Color &color,
                std::function<void(int, int, const Color &)> putPixel) {
  int x, y, dx, dy, d, incE, incNE;
  // ajustamos el punto inicial y terminal a conveniencia para usar
  // el caso base (una linea de entre 0 a 45 grados)
  Point _start{0, 0};
  Point _end{end.x - start.x, end.y - start.y};
  // realizamos transformaciones lineales a las coordendas
  // dependiendo en que cuadrante esten
  bool flipx = _end.x < 0;
  bool flipy = _end.y < 0;
  bool flipcoords = std::abs(_end.x) < std::abs(_end.y);
  if (flipx) {
    _end.x = -_end.x;
  }
  if (flipy) {
    _end.y = -_end.y;
  }
  if (flipcoords) {
    int aux = _end.x;
    _end.x = _end.y;
    _end.y = aux;
  }

  dx = _end.x - _start.x;
  dy = _end.y - _start.y;
  d = dx - 2 * dy;
  incE = -dy;
  incNE = dx - dy;
  if (dy < 0) {
    incE = dy;
    incNE = dx + dy;
  }
  x = 0;
  y = 0;

  putPixel(start.x, start.y, color);

  for (; x < _end.x; x++) {
    if (d <= 0) {
      d += incNE;
      y++;
    } else {
      d += incE;
    }
    int _x = x;
    int _y = y;

    // Desacemos las transformaciones lineales
    // se tienen que deshacer en el orden inverso
    if (flipcoords) {
      int aux = _x;
      _x = _y;
      _y = aux;
    }
    if (flipx) {
      _x = -_x;
    }
    if (flipy) {
      _y = -_y;
    }

    putPixel(start.x + _x, start.y + _y, color);
  }
}

void deploySquare(Point vrtx1, Point vrtx2, const Color &color,
                  std::function<void(int, int, const Color &)> putPixel) {
  deployLine(Point(vrtx1.x, vrtx1.y), Point(vrtx2.x, vrtx1.y), color, putPixel);
  deployLine(Point(vrtx1.x, vrtx1.y), Point(vrtx1.x, vrtx2.y), color, putPixel);
  deployLine(Point(vrtx1.x, vrtx2.y), Point(vrtx2.x, vrtx2.y), color, putPixel);
  deployLine(Point(vrtx2.x, vrtx1.y), Point(vrtx2.x, vrtx2.y), color, putPixel);
}

void deployFilledSquare(Point vrtx1, Point vrtx2, const Color &color,
                        std::function<void(int, int, const Color &)> putPixel) {
  int minX = std::min(vrtx1.x, vrtx2.x);
  int maxX = std::max(vrtx1.x, vrtx2.x);
  int minY = std::min(vrtx1.y, vrtx2.y);
  int maxY = std::max(vrtx1.y, vrtx2.y);

  for (int y = minY; y <= maxY; y++) {
    for (int x = minX; x <= maxX; x++) {
      putPixel(x, y, color);
    }
  }
}

long long edgeFunction(Point a, Point b, Point p) {
  return static_cast<long long>(p.x - a.x) * (b.y - a.y) -
         static_cast<long long>(p.y - a.y) * (b.x - a.x);
}

void deployFilledTriangle(
    Point vrtx1, Point vrtx2, Point vrtx3, const Color &lineColor,
    const Color &fillColor, bool fill,
    std::function<void(int, int, const Color &)> putPixel, bool skipFirstLine) {
  int minX = std::min({vrtx1.x, vrtx2.x, vrtx3.x});
  int maxX = std::max({vrtx1.x, vrtx2.x, vrtx3.x});
  int minY = std::min({vrtx1.y, vrtx2.y, vrtx3.y});
  int maxY = std::max({vrtx1.y, vrtx2.y, vrtx3.y});

  long long area = edgeFunction(vrtx1, vrtx2, vrtx3);

  if (fill) {
    for (int y = minY; y <= maxY; y++) {
      for (int x = minX; x <= maxX; x++) {
        Point p{x, y};
        long long w1 = edgeFunction(vrtx1, vrtx2, p);
        long long w2 = edgeFunction(vrtx2, vrtx3, p);
        long long w3 = edgeFunction(vrtx3, vrtx1, p);

        bool inside = area > 0 ? w1 >= 0 && w2 >= 0 && w3 >= 0
                               : w1 <= 0 && w2 <= 0 && w3 <= 0;
        if (inside) {
          putPixel(x, y, fillColor);
        }
      }
    }
  }

  if(!skipFirstLine) {
    deployLine(vrtx1, vrtx2, lineColor, putPixel);
  }
  deployLine(vrtx2, vrtx3, lineColor, putPixel);
  deployLine(vrtx3, vrtx1, lineColor, putPixel);
}

void drawCirclePoints(int x, int y, int centerx, int centery,
                      const Color &color,
                      std::function<void(int, int, const Color &)> putPixel) {

  putPixel(centerx + x, centery + y, color);
  putPixel(centerx + x, centery - y, color);
  putPixel(centerx - x, centery + y, color);
  putPixel(centerx - x, centery - y, color);
  putPixel(centerx + y, centery + x, color);
  putPixel(centerx + y, centery - x, color);
  putPixel(centerx - y, centery + x, color);
  putPixel(centerx - y, centery - x, color);
}

void deployCircle(Point center, int radius, const Color &color,
                  std::function<void(int, int, const Color &)> putPixel) {
  int dx = center.x;
  int dy = center.y;
  int r = radius;

  int x = 0;
  int y = r;
  int d = 1 - r;

  drawCirclePoints(x, y, center.x, center.y, color, putPixel);
  while (y > x) {
    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
    drawCirclePoints(x, y, center.x, center.y, color, putPixel);
  }
}

void drawEllipsePoints(int x, int y, const Color &c, const Color &fc, bool fill,
                       int centerx, int centery, int flipcoords,
                       std::function<void(int, int, const Color &)> putPixel) {

  if (flipcoords) {
    int aux = x;
    x = y;
    y = aux;
  }
  if (fill) {
    deployLine(Point{centerx + x, centery + y}, Point{centerx - x, centery + y},
               fc, putPixel);
    deployLine(Point{centerx + x, centery - y}, Point{centerx - x, centery - y},
               fc, putPixel);
  }
  putPixel(centerx + x, centery + y, c);
  putPixel(centerx + x, centery - y, c);
  putPixel(centerx - x, centery - y, c);
  putPixel(centerx - x, centery + y, c);
}

void deployEllipse(Point vrtx1, Point vrtx2, const Color &lineColor,
                   const Color &fillColor, bool fill,
                   std::function<void(int, int, const Color &)> putPixel) {
  int centerx = (vrtx2.x + vrtx1.x) / 2;
  int centery = (vrtx2.y + vrtx1.y) / 2;
  int a = std::abs(vrtx1.x - vrtx2.x) / 2;
  int b = std::abs(vrtx1.y - vrtx2.y) / 2;
  bool flipcoords = false;
  if (a < b) {
    flipcoords = true;
    int aux = a;
    a = b;
    b = aux;
  }
  int x, y, d;
  // Modalidad 1
  x = 0;
  y = b;
  d = b * (4 * b - 4 * a * a) + a * a;
  drawEllipsePoints(x, y, lineColor, fillColor, fill, centerx, centery,
                    flipcoords, putPixel);
  bool f = false;
  while (b * b * 2 * (x + 1) < a * a * (2 * y - 1)) {
    if (d < 0) {
      d += 4 * (b * b * (2 * x + 3));
      f = false;
    } else {
      d += 4 * (b * b * (2 * x + 3) + a * a * (-2 * y + 2));
      y--;
      f = true;
    }
    x++;
    drawEllipsePoints(x, y, lineColor, fillColor, fill && f, centerx, centery,
                      flipcoords, putPixel);
  }
  // Modalidad 2
  while (y > 0) {
    if (d < 0) {
      d += 4 * (b * b * (2 * x + 2) + a * a * (-2 * y + 3));
      x++;
    } else {
      d += 4 * a * a * (-2 * y + 3);
    }
    y--;
    drawEllipsePoints(x, y, lineColor, fillColor, fill, centerx, centery,
                      flipcoords, putPixel);
  }
}
