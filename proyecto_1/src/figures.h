#pragma once

#include "primitives.h"

bool isMouseOverLine(Point vrtx1, Point vrtx2, int x, int y);

long long mouseDetectionEdgeFunction(Point a, Point b, Point p);

bool isMouseInsideTriangle(Point vrtx1, Point vrtx2, Point vrtx3, int x, int y);

bool isMouseOverTriangle(Point vrtx1, Point vrtx2, Point vrtx3, int x, int y,
                         bool filled);

bool isMouseOverRect(Point vrtx1, Point vrtx2, Point vrtx3, Point vrtx4, int x,
                     int y, bool filled);

bool isMouseInsideEllipse(Point vrtx1, Point vrtx2, int x, int y,
                          int tolerance = 0);

bool isMouseOverEllipse(Point vrtx1, Point vrtx2, int x, int y, bool filled);

bool mouseInsideBoundingBox(std::vector<Point> vrtxs, int x, int y);

//////////////////////////////////////////////////////
// Figure classes
//////////////////////////////////////////////////////
class Line : public Figure {
public:
  Line(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::Line;
    maxVertices = 2;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }



    deployLine(vrtxs[0], vrtxs[1], lineColor, putPixel);

    // Show control points

  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverLine(vrtxs[0], vrtxs[1], x, y);
  }
};

class Rect : public Figure {
private:
  bool constrainProportions = false;

  void applyProportionsConstraint() {
    int dx = vrtxs[1].x - vrtxs[0].x;
    int dy = vrtxs[1].y - vrtxs[0].y;
    int side = std::min(std::abs(dx), std::abs(dy));

    vrtxs[1].x = vrtxs[0].x + (dx < 0 ? -side : side);
    vrtxs[1].y = vrtxs[0].y + (dy < 0 ? -side : side);
  }

public:
  Rect(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::Rect;
    maxVertices = 4;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void setConstrainProportions(bool value) { constrainProportions = value; }

  void isMouseOver(int x, int y) {
    mouseOver =
        isMouseOverRect(vrtxs[0], vrtxs[1], vrtxs[2], vrtxs[3], x, y, filled);
  }

  void updateSecondaryPoints() {
    vrtxs[2].x = vrtxs[0].x;
    vrtxs[2].y = vrtxs[1].y;
    vrtxs[3].x = vrtxs[1].x;
    vrtxs[3].y = vrtxs[0].y;
  }

  bool onMouseMove(int x, int y) {
    bool result = Figure::onMouseMove(x, y);
    if (state == FigureState::SelectVertices && selectedVrtx >= 1) {
      if (constrainProportions) {
        applyProportionsConstraint();
      }
      updateSecondaryPoints();
    }
    return result;
  }

  bool onMouseButtonDown(int button, int x, int y) {
    if (button != GLFW_MOUSE_BUTTON_LEFT)
      return false;
    bool wasSelecting = state == FigureState::SelectVertices;
    bool result = Figure::onMouseButtonDown(button, x, y);
    if (state == FigureState::SelectVertices && selectedVrtx == 2) {
      // assign the other vertices with the recent two
      stateMachine(0);
      if (wasSelecting && constrainProportions) {
        applyProportionsConstraint();
      }
      updateSecondaryPoints();
      updateCenterPoint();
    }

    return result;
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }



    // deploy lines because the figure can be edited in any quadrilateral.
    deployFilledTriangle(vrtxs[0], vrtxs[1], vrtxs[2], lineColor, fillColor,
                         filled, putPixel, true);
    deployFilledTriangle(vrtxs[0], vrtxs[1], vrtxs[3], lineColor, fillColor,
                         filled, putPixel, true);

    // Show control points

  }
};

class Triangle : public Figure {
public:
  Triangle(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::Triangle;
    maxVertices = 3;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverTriangle(vrtxs[0], vrtxs[1], vrtxs[2], x, y, filled);
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }



    // deploy lines because the figure can be edited in any quadrilateral.
    deployFilledTriangle(vrtxs[0], vrtxs[1], vrtxs[2], lineColor, fillColor,
                         filled, putPixel);

    // Show control points

  }
};

class Ellipse : public Figure {
private:
  bool constrainProportions = false;

  void applyProportionsConstraint() {
    int dx = vrtxs[1].x - vrtxs[0].x;
    int dy = vrtxs[1].y - vrtxs[0].y;
    int side = std::min(std::abs(dx), std::abs(dy));

    vrtxs[1].x = vrtxs[0].x + (dx < 0 ? -side : side);
    vrtxs[1].y = vrtxs[0].y + (dy < 0 ? -side : side);
  }

public:
  Ellipse(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::Ellipse;
    maxVertices = 2;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void setConstrainProportions(bool value) { constrainProportions = value; }

  bool onMouseMove(int x, int y) {
    bool result = Figure::onMouseMove(x, y);
    if (state == FigureState::SelectVertices && selectedVrtx >= 1 &&
        constrainProportions) {
      applyProportionsConstraint();
    }
    return result;
  }

  bool onMouseButtonDown(int button, int x, int y) {
    bool wasSelecting = state == FigureState::SelectVertices && selectedVrtx >= 1;
    bool result = Figure::onMouseButtonDown(button, x, y);
    if (wasSelecting && constrainProportions) {
      applyProportionsConstraint();
      updateCenterPoint();
    }
    return result;
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverEllipse(vrtxs[0], vrtxs[1], x, y, filled);
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }


    // deploy lines because the figure can be edited in any quadrilateral.
    if (std::abs(vrtxs[0].x - vrtxs[1].x) <= 4 ||
        std::abs(vrtxs[0].y - vrtxs[1].y) <= 4) {
      deployLine(vrtxs[0], vrtxs[1], lineColor, putPixel);
    } else {
      deployEllipse(vrtxs[0], vrtxs[1], lineColor, fillColor, filled, putPixel);
    }

  }
};

class BezierCurve : public Figure {
private:
  Color boxColor{0.5, 0.5, 0.5};
  int minTSteps = 40;
  bool curveDirty = true;
  std::vector<Point> curvePoints;

  int getActiveControlPointCount() {
    if (state == FigureState::SelectVertices) {
      return selectedVrtx + 1;
    }
    return maxVertices;
  }

  int getCurveStepCount(int controlPointCount) {
    return std::max(minTSteps, controlPointCount * 4);
  }

  void ensureSelectionPreviewPoint(int x, int y) {
    if (selectedVrtx >= static_cast<int>(vrtxs.size())) {
      vrtxs.push_back(Point{x, y});
      vrtxHover.push_back(false);
    }
  }

  Point calculateBezierPoint(int controlPointCount, float t,
                             std::vector<float> &x, std::vector<float> &y) {
    for (int i = 0; i < controlPointCount; i++) {
      x[i] = static_cast<float>(vrtxs[i].x);
      y[i] = static_cast<float>(vrtxs[i].y);
    }

    for (int level = 1; level < controlPointCount; level++) {
      for (int i = 0; i < controlPointCount - level; i++) {
        x[i] = (1.0f - t) * x[i] + t * x[i + 1];
        y[i] = (1.0f - t) * y[i] + t * y[i + 1];
      }
    }

    return Point{static_cast<int>(x[0]), static_cast<int>(y[0])};
  }

  void updateCurveCache() {
    if (!curveDirty) {
      return;
    }

    int controlPointCount = getActiveControlPointCount();
    if (controlPointCount < 2) {
      curvePoints.clear();
      curveDirty = false;
      return;
    }

    curvePoints.clear();
    int tSteps = getCurveStepCount(controlPointCount);
    curvePoints.reserve(tSteps + 1);
    std::vector<float> x(controlPointCount);
    std::vector<float> y(controlPointCount);
    for (int inc = 0; inc <= tSteps; inc++) {
      float t = static_cast<float>(inc) / static_cast<float>(tSteps);
      curvePoints.push_back(calculateBezierPoint(controlPointCount, t, x, y));
    }

    curveDirty = false;
  }

public:
  BezierCurve(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::BezierCurve;
    maxVertices = 1;
    vrtxs.resize(1);
    vrtxHover.resize(1, false);
  }

  bool elevateGrade() {
    int oldMaxVertices = maxVertices;
    int newMaxVertices = oldMaxVertices + 1;
    std::vector<Point> elevatedVrtxs(newMaxVertices);

    elevatedVrtxs[0] = vrtxs[0];
    elevatedVrtxs[newMaxVertices - 1] = vrtxs[oldMaxVertices - 1];

    for (int i = 1; i < oldMaxVertices; i++) {
      float alpha =
          static_cast<float>(i) / static_cast<float>(newMaxVertices - 1);
      elevatedVrtxs[i].x = static_cast<int>(alpha * vrtxs[i - 1].x +
                                            (1.0f - alpha) * vrtxs[i].x);
      elevatedVrtxs[i].y = static_cast<int>(alpha * vrtxs[i - 1].y +
                                            (1.0f - alpha) * vrtxs[i].y);
    }

    maxVertices = newMaxVertices;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
    for (int i = 0; i < maxVertices; i++) {
      vrtxs[i] = elevatedVrtxs[i];
    }
    updateCenterPoint();
    curveDirty = true;
    return true;
  }

  void translate(int dx, int dy) {
    Figure::translate(dx, dy);
    curveDirty = true;
  }

  bool onMouseButtonDown(int button, int x, int y) {
    if (state == FigureState::SelectVertices) {
      if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (selectedVrtx >= 2) {
          maxVertices = selectedVrtx;
          vrtxs.resize(maxVertices);
          vrtxHover.resize(maxVertices, false);
          updateCenterPoint();
          stateMachine(0);
          curveDirty = true;
          return true;
        }
        return false;
      }

      if (button == GLFW_MOUSE_BUTTON_LEFT) {
        ensureSelectionPreviewPoint(x, y);
        vrtxs[selectedVrtx] = Point{x, y};
        selectedVrtx++;
        ensureSelectionPreviewPoint(x, y);
        vrtxs[selectedVrtx] = Point{x, y};
        curveDirty = true;
        return true;
      }

      return false;
    }

    bool result = Figure::onMouseButtonDown(button, x, y);
    return result;
  }

  bool onMouseMove(int x, int y) {
    if (state == FigureState::SelectVertices) {
      ensureSelectionPreviewPoint(x, y);
      vrtxs[selectedVrtx] = Point{x, y};
      curveDirty = true;
      return true;
    }

    bool wasChangingCurve = state == FigureState::SelectVertices ||
                            state == FigureState::DragVertex ||
                            state == FigureState::DragFigure;
    bool result = Figure::onMouseMove(x, y);
    if (wasChangingCurve && result) {
      curveDirty = true;
    }
    return result;
  }

  bool onMouseButtonUp(int button, int x, int y) {
    bool wasChangingCurve =
        state == FigureState::DragVertex || state == FigureState::DragFigure;
    bool result = Figure::onMouseButtonUp(button, x, y);
    if (wasChangingCurve) {
      curveDirty = true;
    }
    return result;
  }

  void isMouseOver(int x, int y) {
    updateCurveCache();
    mouseOver = false;
    for (size_t i = 1; i < curvePoints.size(); i++) {
      if (isMouseOverLine(curvePoints[i - 1], curvePoints[i], x, y)) {
        mouseOver = true;
        return;
      }
    }
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node


    if (state == FigureState::Selected || state == FigureState::DragVertex ||
        state == FigureState::SelectVertices) {
      int controlPointCount = getActiveControlPointCount();
      for (int i = 1; i < controlPointCount; i++) {
        deployLine(vrtxs[i - 1], vrtxs[i], boxColor, putPixel);
      }
    }

    updateCurveCache();
    for (size_t i = 1; i < curvePoints.size(); i++) {
      deployLine(curvePoints[i - 1], curvePoints[i], lineColor, putPixel);
    }


  }
};
