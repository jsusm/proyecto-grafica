#include "quadTree.h"
#include <algorithm>

QuadTree::QuadTree(QuadTreeRect bounds, int depth)
    : bounds(bounds), depth(depth) {}

bool QuadTree::hasChildren() const { return children[0] != nullptr; }

bool QuadTree::containsPoint(int x, int y) const {
  return x >= bounds.x && x < bounds.x + bounds.width && y >= bounds.y &&
         y < bounds.y + bounds.height;
}

bool QuadTree::intersects(const BoundingBox &box) const {
  int left = bounds.x;
  int right = bounds.x + bounds.width;
  int top = bounds.y;
  int bottom = bounds.y + bounds.height;

  return box.vrtx1.x <= right && box.vrtx2.x >= left && box.vrtx1.y <= bottom &&
         box.vrtx2.y >= top;
}

void QuadTree::subdivide() {
  int halfWidth = bounds.width / 2;
  int halfHeight = bounds.height / 2;
  int rightWidth = bounds.width - halfWidth;
  int bottomHeight = bounds.height - halfHeight;

  children[0] = std::make_unique<QuadTree>(
      QuadTreeRect{bounds.x, bounds.y, halfWidth, halfHeight}, depth + 1);
  children[1] = std::make_unique<QuadTree>(
      QuadTreeRect{bounds.x + halfWidth, bounds.y, rightWidth, halfHeight},
      depth + 1);
  children[2] = std::make_unique<QuadTree>(
      QuadTreeRect{bounds.x, bounds.y + halfHeight, halfWidth, bottomHeight},
      depth + 1);
  children[3] = std::make_unique<QuadTree>(
      QuadTreeRect{bounds.x + halfWidth, bounds.y + halfHeight, rightWidth,
                   bottomHeight},
      depth + 1);
}

void QuadTree::clear() {
  figures.clear();
  for (auto &child : children) {
    child.reset();
  }
}

void QuadTree::insert(Figure *figure) {
  if (figure == nullptr) {
    return;
  }

  BoundingBox box = figure->getBoundingBox();
  if (!intersects(box)) {
    return;
  }

  if (hasChildren()) {
    for (auto &child : children) {
      if (child->intersects(box)) {
        child->insert(figure);
      }
    }
    return;
  }

  figures.push_back(figure);
  if (figures.size() <= maxFiguresPerLeaf || depth >= maxDepth) {
    return;
  }

  std::vector<Figure *> figuresToReinsert = figures;
  figures.clear();
  subdivide();
  for (Figure *storedFigure : figuresToReinsert) {
    insert(storedFigure);
  }
}

std::vector<Figure *> QuadTree::queryPoint(int x, int y) const {
  if (!containsPoint(x, y)) {
    return {};
  }

  if (!hasChildren()) {
    return figures;
  }

  for (const auto &child : children) {
    if (child->containsPoint(x, y)) {
      return child->queryPoint(x, y);
    }
  }

  return {};
}

Color QuadTree::leafColor(int index) const {
  static const std::array<Color, 8> colors{
      Color{1.0f, 0.2f, 0.2f}, Color{0.2f, 1.0f, 0.2f},
      Color{0.2f, 0.4f, 1.0f}, Color{1.0f, 1.0f, 0.2f},
      Color{1.0f, 0.2f, 1.0f}, Color{0.2f, 1.0f, 1.0f},
      Color{1.0f, 0.6f, 0.2f}, Color{0.7f, 0.4f, 1.0f},
  };
  return colors[index % colors.size()];
}

void QuadTree::drawLeaves(
    std::function<void(int, int, const Color &)> putPixel,
    int &leafIndex) const {
  if (hasChildren()) {
    for (const auto &child : children) {
      child->drawLeaves(putPixel, leafIndex);
    }
    return;
  }

  Color color = leafColor(leafIndex++);
  Point topLeft{bounds.x, bounds.y};
  Point bottomRight{bounds.x + bounds.width - 1, bounds.y + bounds.height - 1};
  deployRect(topLeft, bottomRight, color, putPixel);
}

void QuadTree::drawLeaves(
    std::function<void(int, int, const Color &)> putPixel) const {
  int leafIndex = 0;
  drawLeaves(putPixel, leafIndex);
}
