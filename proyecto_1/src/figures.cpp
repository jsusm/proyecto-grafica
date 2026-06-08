#include "primitives.h"
#include "figures.h"

bool isMouseOverLine(Point vrtx1, Point vrtx2, int x, int y) {
  const double tolerance = 8.0;
  double dx = static_cast<double>(vrtx2.x - vrtx1.x);
  double dy = static_cast<double>(vrtx2.y - vrtx1.y);
  double px = static_cast<double>(x - vrtx1.x);
  double py = static_cast<double>(y - vrtx1.y);
  double lengthSquared = dx * dx + dy * dy;

  if (lengthSquared == 0.0) {
    return px * px + py * py <= tolerance * tolerance;
  }

  double t = (px * dx + py * dy) / lengthSquared;
  t = std::max(0.0, std::min(1.0, t));

  double closestX = static_cast<double>(vrtx1.x) + t * dx;
  double closestY = static_cast<double>(vrtx1.y) + t * dy;
  double distanceX = static_cast<double>(x) - closestX;
  double distanceY = static_cast<double>(y) - closestY;

  return distanceX * distanceX + distanceY * distanceY <= tolerance * tolerance;
}

long long mouseDetectionEdgeFunction(Point a, Point b, Point p) {
  return static_cast<long long>(p.x - a.x) * (b.y - a.y) -
         static_cast<long long>(p.y - a.y) * (b.x - a.x);
}

bool isMouseInsideTriangle(Point vrtx1, Point vrtx2, Point vrtx3, int x,
                           int y) {
  Point p{x, y};
  long long area = mouseDetectionEdgeFunction(vrtx1, vrtx2, vrtx3);
  long long w1 = mouseDetectionEdgeFunction(vrtx1, vrtx2, p);
  long long w2 = mouseDetectionEdgeFunction(vrtx2, vrtx3, p);
  long long w3 = mouseDetectionEdgeFunction(vrtx3, vrtx1, p);

  if (area == 0) {
    return isMouseOverLine(vrtx1, vrtx2, x, y) ||
           isMouseOverLine(vrtx2, vrtx3, x, y) ||
           isMouseOverLine(vrtx3, vrtx1, x, y);
  }

  return area > 0 ? w1 >= 0 && w2 >= 0 && w3 >= 0
                  : w1 <= 0 && w2 <= 0 && w3 <= 0;
}

bool isMouseOverTriangle(Point vrtx1, Point vrtx2, Point vrtx3, int x, int y,
                         bool filled) {
  if (filled && isMouseInsideTriangle(vrtx1, vrtx2, vrtx3, x, y)) {
    return true;
  }

  return isMouseOverLine(vrtx1, vrtx2, x, y) ||
         isMouseOverLine(vrtx1, vrtx3, x, y) ||
         isMouseOverLine(vrtx2, vrtx3, x, y);
}

bool isMouseOverRect(Point vrtx1, Point vrtx2, Point vrtx3, Point vrtx4, int x,
                     int y, bool filled) {
  if (filled && (isMouseInsideTriangle(vrtx1, vrtx2, vrtx3, x, y) ||
                 isMouseInsideTriangle(vrtx1, vrtx2, vrtx4, x, y))) {
    return true;
  }

  return isMouseOverLine(vrtx1, vrtx3, x, y) ||
         isMouseOverLine(vrtx1, vrtx4, x, y) ||
         isMouseOverLine(vrtx2, vrtx3, x, y) ||
         isMouseOverLine(vrtx2, vrtx4, x, y);
}

bool isMouseInsideEllipse(Point vrtx1, Point vrtx2, int x, int y,
                          int tolerance) {
  int centerX = (vrtx1.x + vrtx2.x) / 2;
  int centerY = (vrtx1.y + vrtx2.y) / 2;
  long long radiusX = std::abs(vrtx1.x - vrtx2.x) / 2 + tolerance;
  long long radiusY = std::abs(vrtx1.y - vrtx2.y) / 2 + tolerance;

  if (radiusX <= 0 && radiusY <= 0) {
    return x == centerX && y == centerY;
  }
  if (radiusX <= 0 || radiusY <= 0) {
    return isMouseOverLine(vrtx1, vrtx2, x, y);
  }

  signed long long _x = x - centerX;
  signed long long _y = y - centerY;

  return radiusY * radiusY * _x * _x + radiusX * radiusX * _y * _y <=
         radiusX * radiusX * radiusY * radiusY;
}

bool isMouseOverEllipse(Point vrtx1, Point vrtx2, int x, int y, bool filled) {
  int tolerance = 3;

  if (filled && isMouseInsideEllipse(vrtx1, vrtx2, x, y)) {
    return true;
  }

  int radiusX = std::abs(vrtx1.x - vrtx2.x) / 2;
  int radiusY = std::abs(vrtx1.y - vrtx2.y) / 2;
  if (radiusX < tolerance || radiusY < tolerance) {
    return isMouseOverLine(vrtx1, vrtx2, x, y);
  }

  return isMouseInsideEllipse(vrtx1, vrtx2, x, y, tolerance) &&
         !isMouseInsideEllipse(vrtx1, vrtx2, x, y, -tolerance);
}

bool mouseInsideBoundingBox(std::vector<Point> vrtxs, int x, int y) {
  int minX = vrtxs[0].x;
  int minY = vrtxs[0].y;
  int maxX = vrtxs[0].x;
  int maxY = vrtxs[0].y;
  for (auto &p : vrtxs) {
    minX = std::min(minX, p.x);
    minY = std::min(minY, p.y);
    maxX = std::max(maxX, p.x);
    maxY = std::max(maxY, p.y);
  }
  return x >= minX && x <= maxX && y >= minY && y <= maxY;
}
