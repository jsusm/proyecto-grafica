#include "primitives.h"
#include <algorithm>
#include <cmath>
#include <functional>

void deployLine(Point start, Point end, const Color &color,
                std::function<void(int, int, const Color &)> putPixel) {
  if (start.y == end.y) {
    int minX = std::min(start.x, end.x);
    int maxX = std::max(start.x, end.x);
    for (int x = minX; x <= maxX; x++) {
      putPixel(x, start.y, color);
    }
    return;
  }

  if (start.x == end.x) {
    int minY = std::min(start.y, end.y);
    int maxY = std::max(start.y, end.y);
    for (int y = minY; y <= maxY; y++) {
      putPixel(start.x, y, color);
    }
    return;
  }

  int x = start.x;
  int y = start.y;
  int dx = std::abs(end.x - start.x);
  int dy = std::abs(end.y - start.y);
  int stepX = start.x < end.x ? 1 : -1;
  int stepY = start.y < end.y ? 1 : -1;
  int error = dx - dy;

  while (true) {
    putPixel(x, y, color);

    if (x == end.x && y == end.y) {
      break;
    }

    int doubleError = 2 * error;
    if (doubleError > -dy) {
      error -= dy;
      x += stepX;
    }
    if (doubleError < dx) {
      error += dx;
      y += stepY;
    }
  }
}

void deployRect(Point vrtx1, Point vrtx2, const Color &color,
                  std::function<void(int, int, const Color &)> putPixel) {
  deployLine(Point(vrtx1.x, vrtx1.y), Point(vrtx2.x, vrtx1.y), color, putPixel);
  deployLine(Point(vrtx1.x, vrtx1.y), Point(vrtx1.x, vrtx2.y), color, putPixel);
  deployLine(Point(vrtx1.x, vrtx2.y), Point(vrtx2.x, vrtx2.y), color, putPixel);
  deployLine(Point(vrtx2.x, vrtx1.y), Point(vrtx2.x, vrtx2.y), color, putPixel);
}

void deployFilledRect(Point vrtx1, Point vrtx2, const Color &color,
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

void deployFilledTriangle(Point vrtx1, Point vrtx2, Point vrtx3,
                          const Color &lineColor, const Color &fillColor,
                          bool fill,
                          std::function<void(int, int, const Color &)> putPixel,
                          bool skipFirstLine) {
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

  if (!skipFirstLine) {
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

void drawCircleFillLines(int x, int y, int centerx, int centery,
                         const Color &color,
                         std::function<void(int, int, const Color &)> putPixel) {
  deployLine(Point{centerx - x, centery + y}, Point{centerx + x, centery + y},
             color, putPixel);
  deployLine(Point{centerx - x, centery - y}, Point{centerx + x, centery - y},
             color, putPixel);
  deployLine(Point{centerx - y, centery + x}, Point{centerx + y, centery + x},
             color, putPixel);
  deployLine(Point{centerx - y, centery - x}, Point{centerx + y, centery - x},
             color, putPixel);
}

void deployCircle(Point center, int radius, const Color &lineColor,
                  const Color &fillColor, bool fill,
                  std::function<void(int, int, const Color &)> putPixel) {
  int r = radius;

  int x = 0;
  int y = r;
  int d = 1 - r;

  if (fill) {
    drawCircleFillLines(x, y, center.x, center.y, fillColor, putPixel);
  }
  drawCirclePoints(x, y, center.x, center.y, lineColor, putPixel);
  while (y > x) {
    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * (x - y) + 5;
      y--;
    }
    x++;
    if (fill) {
      drawCircleFillLines(x, y, center.x, center.y, fillColor, putPixel);
    }
    drawCirclePoints(x, y, center.x, center.y, lineColor, putPixel);
  }
}

void drawEllipsePoints(int x, int y, const Color &c, const Color &fc, bool fill,
                       int centerx, int centery, int flipcoords,
                       std::function<void(int, int, const Color &)> putPixel,
                       int a, int b) {

  if (flipcoords) {
    int aux = x;
    x = y;
    y = aux;
  }
  if (fill) {
    if (!flipcoords) {
      deployLine(Point{centerx + x, centery + y},
                 Point{centerx - x, centery + y}, fc, putPixel);
      deployLine(Point{centerx + x, centery - y},
                 Point{centerx - x, centery - y}, fc, putPixel);
    } else {
      deployLine(Point{centerx + x, centery + y},
                 Point{centerx + x, centery - y}, fc, putPixel);
      deployLine(Point{centerx - x, centery + y},
                 Point{centerx - x, centery - y}, fc, putPixel);
    }
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
                    flipcoords, putPixel, a, b);
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
                      flipcoords, putPixel, a, b);
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
                      flipcoords, putPixel, b, a);
  }
}
