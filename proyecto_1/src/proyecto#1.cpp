#include "engine2D.h"
#include "imgui.h"
#include "persistFigures.h"
#include "primitives.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <vector>

//////////////////////////////////////
// Mouse Detection
//////////////////////////////////////
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
                          int tolerance = 0) {
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

    drawBoundingBox(putPixel);

    deployLine(vrtxs[0], vrtxs[1], lineColor, putPixel);

    // Show control points
    drawControlPoints(putPixel);
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverLine(vrtxs[0], vrtxs[1], x, y);
  }
};

class Rect : public Figure {
public:
  Rect(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::Rect;
    maxVertices = 4;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

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
      updateSecondaryPoints();
    }
    return result;
  }

  bool onMouseButtonDown(int button, int x, int y) {
    if (button != GLFW_MOUSE_BUTTON_LEFT)
      return false;
    bool result = Figure::onMouseButtonDown(button, x, y);
    if (state == FigureState::SelectVertices && selectedVrtx == 2) {
      // assign the other vertices with the recent two
      stateMachine(0);
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

    drawBoundingBox(putPixel);

    // deploy lines because the figure can be edited in any quadrilateral.
    deployFilledTriangle(vrtxs[0], vrtxs[1], vrtxs[2], lineColor, fillColor,
                         filled, putPixel, true);
    deployFilledTriangle(vrtxs[0], vrtxs[1], vrtxs[3], lineColor, fillColor,
                         filled, putPixel, true);

    // Show control points
    drawControlPoints(putPixel);
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

    drawBoundingBox(putPixel);

    // deploy lines because the figure can be edited in any quadrilateral.
    deployFilledTriangle(vrtxs[0], vrtxs[1], vrtxs[2], lineColor, fillColor,
                         filled, putPixel);

    // Show control points
    drawControlPoints(putPixel);
  }
};

class Ellipse : public Figure {
public:
  Ellipse(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::Ellipse;
    maxVertices = 2;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverEllipse(vrtxs[0], vrtxs[1], x, y, filled);
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }

    drawBoundingBox(putPixel);
    // deploy lines because the figure can be edited in any quadrilateral.
    if (std::abs(vrtxs[0].x - vrtxs[1].x) <= 4 ||
        std::abs(vrtxs[0].y - vrtxs[1].y) <= 4) {
      deployLine(vrtxs[0], vrtxs[1], lineColor, putPixel);
    } else {
      deployEllipse(vrtxs[0], vrtxs[1], lineColor, fillColor, filled, putPixel);
    }
    drawControlPoints(putPixel);
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
    drawBoundingBox(putPixel);

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

    drawControlPoints(putPixel);
  }
};

//////////////////////////////////////////////////////
// Figure Interface and Coordinator
//////////////////////////////////////////////////////
enum class ToolsType { Line, Select, Triangle, Ellipse, Rect, BezierCurve };

int WindowWidth = 1400;
int WindowHeight = 720;
int ToolsWindowWidth = 300;

class proyecto1 : public Engine2D {
private:
  Color backgroundColor = Color(0.1f, 0.1f, 0.15f);
  Color colorPincel = Color(1.0f, 0.0f, 0.0f);
  bool dibujando = false;
  bool isHover = false;

public:
  proyecto1()
      : Engine2D(1400, 720,
                 "Proyecto #1 - Gestion y Despliegue de Primitivas") {}

  // points to the figure currently focused
  Figure *currentFigure = nullptr;
  // The list of figures
  std::vector<std::unique_ptr<Figure>> figures{};

  // saves which tool the user is using
  ToolsType currentTool = ToolsType::Select;

  bool mouseReserved = false;

  Color lineColor{1, 1, 1};
  Color fillColor{1, 1, 1};
  bool filled = false;

  void setup() override {
    clear(backgroundColor);
    std::cout << "Motor inicializado exitosamente." << std::endl;
    SerializedFiguresFile fileFigures = loadFigures();
    for (auto &f : fileFigures.figures) {
      auto figureType = static_cast<FigureType>(f.type);
      switch (figureType) {
      case FigureType::Line:
        figures.push_back(std::make_unique<Line>(f.lineColor, f.fillColor));
        break;
      case FigureType::Rect:
        figures.push_back(std::make_unique<Rect>(f.lineColor, f.fillColor));
        break;
      case FigureType::Triangle:
        figures.push_back(std::make_unique<Triangle>(f.lineColor, f.fillColor));
        break;
      case FigureType::Ellipse:
        figures.push_back(std::make_unique<Ellipse>(f.lineColor, f.fillColor));
        break;
      case FigureType::BezierCurve:
        figures.push_back(
            std::make_unique<BezierCurve>(f.lineColor, f.fillColor));
        break;
      }
      currentFigure = figures.back().get();
      currentFigure->setFilled(f.filled);
      currentFigure->loadVrtxs(f.points);
    }
  }

  SerializedFiguresFile collectSerializedFigures() {
    SerializedFiguresFile serializedFiguresFile{};
    serializedFiguresFile.figures.reserve(figures.size());

    for (const auto &figure : figures) {
      SerializedFigure serializedFigure{};
      serializedFigure.type = figure->type;
      serializedFigure.lineColor = figure->getLineColor();
      serializedFigure.fillColor = figure->getFillColor();
      serializedFigure.filled = figure->isFilled();
      serializedFigure.points = figure->getVrtxs();
      serializedFigure.nPoints = serializedFigure.points.size();

      serializedFiguresFile.figures.push_back(serializedFigure);
    }

    serializedFiguresFile.nFigures = serializedFiguresFile.figures.size();
    return serializedFiguresFile;
  }

  // Eventos
  void onkeyDown(int key) override {
    if (key == GLFW_KEY_Q) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE) {
      clear(backgroundColor);
    }
  }
  void onMouseButtonDown(int button, double x, double y) override {
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentTool == ToolsType::Line) {

      figures.push_back(std::make_unique<Line>(lineColor, fillColor));
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(button, x, y);
      currentFigure->setFilled(filled);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Rect) {

      figures.push_back(std::make_unique<Rect>(lineColor, fillColor));
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(button, x, y);
      currentFigure->setFilled(filled);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Triangle) {

      figures.push_back(std::make_unique<Triangle>(lineColor, fillColor));
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(button, x, y);
      currentFigure->setFilled(filled);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Ellipse) {

      figures.push_back(std::make_unique<Ellipse>(lineColor, fillColor));
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(button, x, y);
      currentFigure->setFilled(filled);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::BezierCurve) {

      figures.push_back(std::make_unique<BezierCurve>(lineColor, fillColor));
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(button, x, y);
      currentFigure->setFilled(filled);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Select)

    {

      if (currentFigure == nullptr) {
        for (auto it = figures.rbegin(); it != figures.rend(); ++it) {
          (*it)->onMouseButtonDown(button, x, y);

          if ((*it)->mouseOver) {
            currentFigure = it->get();
            currentFigure->select();
            onSelectFigure();
            break;
          }
        }
      } else {
        mouseReserved = currentFigure->onMouseButtonDown(button, x, y);
        if (currentTool == ToolsType::Select) {
          if (!mouseReserved) {
            currentFigure->unselect();
            currentFigure = nullptr;
            for (auto it = figures.rbegin(); it != figures.rend(); ++it) {
              (*it)->onMouseButtonDown(button, x, y);

              if ((*it)->mouseOver) {
                currentFigure = it->get();
                currentFigure->select();
                onSelectFigure();
                break;
              }
            }
          }
        }
      }
    }
  }

  void onSelectFigure() {
    lineColor = currentFigure->getLineColor();
    if (currentFigure->isFilled()) {
      fillColor = currentFigure->getFillColor();
    }
  }

  void onMouseButtonUp(int button, double x, double y) override {
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentFigure != nullptr) {
      mouseReserved = currentFigure->onMouseButtonUp(button, x, y);
      if (!mouseReserved) {
        for (auto &f : figures) {
          f->onMouseButtonUp(button, x, y);
        }
      }
    }
  }

  void onMouseMove(double x, double y) override {
    // do not register the event if the mouse is over the tools window
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    isHover = false;
    if (currentFigure != nullptr) {
      mouseReserved = currentFigure->onMouseMove(x, y);
      if (!mouseReserved) {
        for (auto &f : figures) {
          f->onMouseMove(x, y);
          isHover |= f->mouseOver;
        }
      }
    } else {
      for (auto &f : figures) {
        f->onMouseMove(x, y);
        isHover |= f->mouseOver;
      }
    }
  }

  void update(float deltaTime) override {
    clear(backgroundColor);
    for (auto &f : figures) {
      f->draw(
          [this](int x, int y, const Color &color) { putPixel(x, y, color); });
    }
  }

  void toolsSection() {
    ImGui::SeparatorText("Herramientas");
    if (ImGui::Button("Select")) {
      currentTool = ToolsType::Select;
    }
    ImGui::SameLine();
    if (ImGui::Button("Line")) {
      currentTool = ToolsType::Line;
      if (currentFigure != nullptr) {
        currentFigure->unselect();
        currentFigure = nullptr;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Rect")) {
      currentTool = ToolsType::Rect;
      if (currentFigure != nullptr) {
        currentFigure->unselect();
        currentFigure = nullptr;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Triangle")) {
      currentTool = ToolsType::Triangle;
      if (currentFigure != nullptr) {
        currentFigure->unselect();
        currentFigure = nullptr;
      }
    }
    if (ImGui::Button("Ellipse")) {
      currentTool = ToolsType::Ellipse;
      if (currentFigure != nullptr) {
        currentFigure->unselect();
        currentFigure = nullptr;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Curva de Bezier")) {
      currentTool = ToolsType::BezierCurve;
    }
  }

  void drawUI() override {
    // ImGui::ShowDemoWindow();

    // Set up tools window options

    ImGuiWindowFlags window_flags = 0;
    window_flags |= ImGuiWindowFlags_NoMove;
    window_flags |= ImGuiWindowFlags_NoResize;
    window_flags |= ImGuiWindowFlags_NoCollapse;
    ImGui::SetNextWindowPos(ImVec2(WindowWidth - ToolsWindowWidth, 0));
    ImGui::SetNextWindowSize(ImVec2(ToolsWindowWidth, WindowHeight));

    // Mouse configuration
    if (isHover) {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    } else {
      ImGui::SetMouseCursor(ImGuiMouseCursor_Arrow);
    }

    // Begin
    ImGui::Begin("Herramientas", NULL, window_flags);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    if(ImGui::Button("Guardar en disco")) {
      saveFigures(collectSerializedFigures());
    }
    ImGui::Separator();
    toolsSection();

    ImGui::SeparatorText("Color de Fondo");
    float bc[3] = {backgroundColor.r, backgroundColor.g, backgroundColor.b};
    if (ImGui::ColorEdit3("", bc)) {
      backgroundColor.r = bc[0];
      backgroundColor.g = bc[1];
      backgroundColor.b = bc[2];
    };

    ImGui::Separator();
    ImGui::SeparatorText("Color de Linea");
    float lc[3] = {lineColor.r, lineColor.g, lineColor.b};
    float fc[3] = {fillColor.r, fillColor.g, fillColor.b};
    if (ImGui::ColorEdit3(" ", lc)) {
      lineColor.r = lc[0];
      lineColor.g = lc[1];
      lineColor.b = lc[2];
      if (currentFigure != nullptr) {
        currentFigure->setLineColor(lineColor);
      }
    };
    ImGui::SeparatorText("Color de relleno");
    if (ImGui::ColorEdit3("  ", fc)) {
      fillColor.r = fc[0];
      fillColor.g = fc[1];
      fillColor.b = fc[2];
      if (currentFigure != nullptr) {
        currentFigure->setFillColor(fillColor);
      }
    };
    if (ImGui::Checkbox("Rellenar figura", &filled)) {
      if (currentFigure != nullptr) {
        currentFigure->setFilled(filled);
      }
    };
    ImGui::SeparatorText("Controles de Figura");
    if (currentFigure != nullptr) {
      if (currentFigure->type == FigureType::BezierCurve) {
        if (ImGui::Button("Elevate Bezier Curve Grade")) {
          BezierCurve *bc = static_cast<BezierCurve *>(currentFigure);
          bc->elevateGrade();
        }
      }
    }
    ImGui::End();
  }
};

int main() {
  proyecto1 app;
  app.run();
  return 0;
}
