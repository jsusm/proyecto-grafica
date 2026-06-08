#pragma once

#include "engine2D.h"
#include <functional>

struct Point {
  int x;
  int y;
};

struct BoundingBox {
  Point vrtx1;
  Point vrtx2;
};

enum class FigureState {
  SelectVertices,
  Selected,
  DragVertex,
  Unselected,
  DragFigure
};

void deployLine(Point start, Point end, const Color &color,
                std::function<void(int, int, const Color &)> putPixel);

void deploySquare(Point vrtx1, Point vrtx2, const Color &color,
                  std::function<void(int, int, const Color &)> putPixel);

void deployFilledSquare(Point vrtx1, Point vrtx2, const Color &color,
                        std::function<void(int, int, const Color &)> putPixel);

void deployFilledTriangle(Point vrtx1, Point vrtx2, Point vrtx3,
                          const Color &lineColor, const Color &fillColor,
                          bool fill,
                          std::function<void(int, int, const Color &)> putPixel,
                          bool skipFirstLine = false);

void deployEllipse(Point vrtx1, Point vrtx2, const Color &lineColor,
                   const Color &fillColor, bool fill,
                   std::function<void(int, int, const Color &)> putPixel);

void deployCircle(Point center, int radius, const Color &lineColor,
                  const Color &fillColor, bool fill,
                  std::function<void(int, int, const Color &)> putPixel);

enum class FigureType { Line, Rect, Triangle, Ellipse, BezierCurve, Unknown };

class Figure {
protected:
  std::vector<Point> vrtxs;
  std::vector<bool> vrtxHover;
  int vrtxRadius = 4;
  int maxVertices = 2;
  int selectedVrtx = 0;
  Color lineColor{1.0f, 1.0f, 1.0f};
  Color fillColor{0.99f, 0.99f, 0.99f};
  bool filled = false;
  bool hoverCenterVrtx = false;
  Point centerVrtx;
  Color boxColor = Color(0.2f, 0.2f, 1.0f);

  Color selectVrtxFill = Color(0.9f, 0.9f, 0.1f);
  Color selectVrtxFillHover = Color(1, 1, 0.4);
  Color selectVrtxLine = Color(1, 1, 1);
  Color selectVrtxLineHover = Color(0.9, 0.9, 1);
  Color selectVrtxLineContrast = Color(0, 0, 0);

  Point dragOrigin;

  FigureState state = FigureState::SelectVertices;

  void stateMachine(int branch) {
    switch (state) {
    case FigureState::SelectVertices:
      if (vrtxs.size() == maxVertices) {
        state = FigureState::Selected;
      }
      break;
    case FigureState::DragVertex:
      state = FigureState::Selected;
      break;
    case FigureState::Selected:
      if (branch == 1) {
        state = FigureState::DragVertex;
      } else if (branch == 2) {
        state = FigureState::DragFigure;
      } else {
        state = FigureState::Unselected;
      }
      break;
    case FigureState::DragFigure:
      state = FigureState::Selected;
      break;
    case FigureState::Unselected:
      state = FigureState::Selected;
      break;
    }
  }

  bool mouseInVrtx(Point vrtx, int x, int y) {
    int dx = vrtx.x - x;
    int dy = vrtx.y - y;
    int distanceSquared = dx * dx + dy * dy;
    return distanceSquared <= vrtxRadius * vrtxRadius;
  }

  void updateVrtxHover(int x, int y) {
    for (int i = 0; i < vrtxs.size(); i++) {
      vrtxHover[i] = mouseInVrtx(vrtxs[i], x, y);
    }
    hoverCenterVrtx = mouseInVrtx(centerVrtx, x, y);
  }

  void updateCenterPoint() {
    int x = 0;
    int y = 0;
    for (Point p : vrtxs) {
      x += p.x;
      y += p.y;
    }
    x /= vrtxs.size();
    y /= vrtxs.size();
    centerVrtx.x = x;
    centerVrtx.y = y;
  }

  void drawBoundingBox(std::function<void(int, int, const Color &)> putPixel) {
    if (state != FigureState::Unselected &&
        state != FigureState::SelectVertices) {
      BoundingBox bb = getBoundingBox();
      deploySquare(Point(bb.vrtx1.x - 4, bb.vrtx1.y - 4),
                   Point(bb.vrtx2.x + 4, bb.vrtx2.y + 4), boxColor, putPixel);
    }
  }

  void
  drawControlPoints(std::function<void(int, int, const Color &)> putPixel) {
    // Show control points
    if (state == FigureState::Selected) {
      int r = vrtxRadius;
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          deployCircle(vrtxs[i], r + 1, selectVrtxLineContrast,
                       selectVrtxLineContrast, false, putPixel);
          deployCircle(vrtxs[i], r, selectVrtxLineHover, selectVrtxFillHover,
                       true, putPixel);
        } else {
          deployCircle(vrtxs[i], r + 1, selectVrtxLineContrast,
                       selectVrtxLineContrast, false, putPixel);
          deployCircle(vrtxs[i], r, selectVrtxLine, selectVrtxFill, true,
                       putPixel);
        }
      }
      if (hoverCenterVrtx) {
        deployCircle(centerVrtx, r + 1, selectVrtxLineContrast,
                     selectVrtxLineContrast, false, putPixel);
        deployCircle(centerVrtx, r, selectVrtxLineHover, selectVrtxFillHover,
                     true, putPixel);
      } else {
        deployCircle(centerVrtx, r + 1, selectVrtxLineContrast,
                     selectVrtxLineContrast, false, putPixel);
        deployCircle(centerVrtx, r, selectVrtxLine, selectVrtxFill, true,
                     putPixel);
      }
    }
  }

  virtual BoundingBox getBoundingBox() {
    // get min x and y
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
    return BoundingBox(Point(minX, minY), Point(maxX, maxY));
  }
  virtual void isMouseOver(int x, int y) {}

public:
  Figure(Color line, Color fill) {
    this->lineColor = line;
    this->fillColor = fill;
  }

  virtual ~Figure() = default;

  FigureType type = FigureType::Unknown;
  bool selected = false;
  bool mouseOver = false;
  virtual void draw(std::function<void(int, int, const Color &)> putPixel) {};

  void setLineColor(Color color) { lineColor = color; }
  Color getLineColor() { return lineColor; }
  void setFillColor(Color color) { fillColor = color; }
  Color getFillColor() { return fillColor; }
  void setFilled(bool value) { filled = value; }
  bool isFilled() { return filled; }

  void select() {
    switch (state) {
    case FigureState::Unselected:
      stateMachine(0);
      break;
    default:
      break;
    }
  }

  void unselect() {
    switch (state) {
    case FigureState::Selected:
      stateMachine(0);
      break;
    default:
      break;
    }
  }

  virtual bool onMouseMove(int x, int y) {
    switch (state) {
    case FigureState::Selected:
      isMouseOver(x, y);
      updateVrtxHover(x, y);
      break;

    case FigureState::Unselected:
      isMouseOver(x, y);
      break;

    case FigureState::SelectVertices:
      vrtxs[selectedVrtx].x = x;
      vrtxs[selectedVrtx].y = y;
      return true;
      break;

    case FigureState::DragVertex:
      vrtxs[selectedVrtx].x = x;
      vrtxs[selectedVrtx].y = y;
      return true;
      break;

    case FigureState::DragFigure:
      updateVrtxHover(x, y);
      int dx = x - dragOrigin.x;
      int dy = y - dragOrigin.y;
      for (int i = 0; i < vrtxs.size(); i++) {
        vrtxs[i].x += dx;
        vrtxs[i].y += dy;
      }
      dragOrigin.x = x;
      dragOrigin.y = y;
      return true;
      break;
    }
    return false;
  }

  virtual bool onMouseButtonDown(int button, int x, int y) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) {
      return false;
    }
    isMouseOver(x, y);
    updateVrtxHover(x, y);
    switch (state) {
    case FigureState::SelectVertices:
      for (int i = selectedVrtx; i < vrtxs.size(); i++) {
        vrtxs[i].x = x;
        vrtxs[i].y = y;
      }
      selectedVrtx++;
      if (selectedVrtx == maxVertices) {
        updateCenterPoint();
        stateMachine(0);
      }
      return true;
      break;
    case FigureState::Selected:
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          selectedVrtx = i;
          stateMachine(1);
          return true;
          break;
        }
      }
      if (hoverCenterVrtx) {
        stateMachine(2);
        dragOrigin.x = x;
        dragOrigin.y = y;
        return true;
        break;
      }
      break;
    case FigureState::DragFigure:
      return true;
      break;
    case FigureState::Unselected:
      isMouseOver(x, y);
      break;
    }
    return false;
  }

  virtual bool onMouseButtonUp(int button, int x, int y) {
    if (button != GLFW_MOUSE_BUTTON_LEFT) {
      return false;
    }
    switch (state) {
    case FigureState::Selected:
      break;
    case FigureState::DragVertex:
      vrtxs[selectedVrtx].x = x;
      vrtxs[selectedVrtx].y = y;
      updateCenterPoint();
      stateMachine(0);
      break;
    case FigureState::DragFigure:
      stateMachine(0);
      updateCenterPoint();
      updateVrtxHover(x, y);
      return true;
      break;

    case FigureState::Unselected:
      isMouseOver(x, y);
      break;
    }
    return false;
  }

  // if this methods return true that means the mouse is "taken"
  // no other figure can interact with the mouse, like when the
  // user is dragging the figure, if return false the mouse
  // can be interacting with other figures
  // virtual bool onMouseMove(int, int) { return false; }
  // virtual bool onMouseButtonDown(int, int) { return false; }
  // virtual bool onMouseButtonUp(int, int) { return false; }
};
