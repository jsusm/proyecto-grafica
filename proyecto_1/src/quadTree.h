#pragma once

#include "primitives.h"
#include <array>
#include <functional>
#include <memory>
#include <vector>

struct QuadTreeRect {
  int x;
  int y;
  int width;
  int height;
};

class QuadTree {
private:
  static constexpr int maxFiguresPerLeaf = 4;
  static constexpr int maxDepth = 6;

  QuadTreeRect bounds;
  int depth;
  std::vector<Figure *> figures;
  std::array<std::unique_ptr<QuadTree>, 4> children;

  bool hasChildren() const;
  bool containsPoint(int x, int y) const;
  bool intersects(const BoundingBox &box) const;
  void subdivide();
  Color leafColor(int index) const;
  void drawLeaves(std::function<void(int, int, const Color &)> putPixel,
                  int &leafIndex) const;

public:
  QuadTree(QuadTreeRect bounds, int depth = 0);

  void clear();
  void insert(Figure *figure);
  std::vector<Figure *> queryPoint(int x, int y) const;
  void drawLeaves(std::function<void(int, int, const Color &)> putPixel) const;
};
