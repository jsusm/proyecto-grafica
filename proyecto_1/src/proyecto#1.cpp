#include "engine2D.h"
#include "imgui.h"
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
  int a, b, c;
  a = vrtx1.y - vrtx2.y;          // -dy
  b = vrtx2.x - vrtx1.x;          // dx
  c = -vrtx1.y * b - vrtx1.x * a; // -y0*dx + x0*dy
  int z = std::abs(a * x + b * y + c);
  int tolerance = 16;
  int maxD = std::max(std::abs(a), std::abs(b));

  int minX = vrtx1.x < vrtx2.x ? vrtx1.x : vrtx2.x;
  int minY = vrtx1.y < vrtx2.y ? vrtx1.y : vrtx2.y;

  int maxX = vrtx1.x >= vrtx2.x ? vrtx1.x : vrtx2.x;
  int maxY = vrtx1.y >= vrtx2.y ? vrtx1.y : vrtx2.y;

  bool mouseInX = x >= minX && x <= maxX;
  bool mouseInY = y >= minY && y <= maxY;
  // if the line is too vertical or too horizontal
  // only check the proximity through the line equation
  if (std::abs(b) < 10) {
    mouseInX = true;
  }
  if (std::abs(a) < 10) {
    mouseInY = true;
  }

  return z < tolerance * maxD && mouseInX && mouseInY;
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
  int maxBezierControlPoints = 10;

public:
  BezierCurve(Color line, Color fill) : Figure(line, fill) {
    type = FigureType::BezierCurve;
    maxVertices = maxBezierControlPoints;
    vrtxs.resize(maxBezierControlPoints);
    vrtxHover.resize(maxVertices, false);
  }

  bool isMouseOverBezierCurve(std::vector<Point> &vrtxs, int x, int y,
                              int level = 0) {
    std::vector<Point> vrtxsLeft;
    std::vector<Point> vrtxsRight;
    // build the
    for (size_t i = 0; i < maxVertices; i++) {
      vrtxsLeft.push_back(calculateCasteljauPoly(i, 0, 0.5, vrtxs));
      vrtxsRight.push_back(
          calculateCasteljauPoly(i, maxVertices - i - 1, 0.5, vrtxs));
    }
    bool mouseInLeft = mouseInsideBoundingBox(vrtxsLeft, x, y);
    bool mouseInRight = mouseInsideBoundingBox(vrtxsRight, x, y);
    if (!mouseInLeft && !mouseInRight)
      return false;
    if (level >= 3)
      return mouseInLeft || mouseInRight;

    if (mouseInLeft) {
      return isMouseOverBezierCurve(vrtxsLeft, x, y, level + 1);
    } else {
      return isMouseOverBezierCurve(vrtxsRight, x, y, level + 1);
    }
  }

  bool onMouseButtonDown(int button, int x, int y) {
    if(button == GLFW_MOUSE_BUTTON_RIGHT) {
      if(state == FigureState::SelectVertices) {
        maxVertices = selectedVrtx;
        updateCenterPoint();
        stateMachine(0);
        return false;
      }
    }
    return Figure::onMouseButtonDown(button, x, y);
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverBezierCurve(vrtxs, x, y);
  }

  Point calculateCasteljauPoly(int deg, int idx, float t,
                               std::vector<Point> &vrtxs) {
    if (deg != 0) {
      Point p1 = calculateCasteljauPoly(deg - 1, idx, t, vrtxs);
      Point p2 = calculateCasteljauPoly(deg - 1, idx + 1, t, vrtxs);
      return Point((1 - t) * p1.x + t * p2.x, (1 - t) * p1.y + t * p2.y);
    }
    return vrtxs[idx];
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    drawBoundingBox(putPixel);

    Point prevPoint = vrtxs[0];

    if (state == FigureState::Selected || state == FigureState::DragVertex ||
        state == FigureState::SelectVertices) {
      for (int i = 1; i < maxVertices; i++) {
        deployLine(vrtxs[i-1], vrtxs[i], boxColor, putPixel);
      }
    }

    prevPoint = vrtxs[0];
    int tSteps = 40;
    for (int inc = 0; inc < tSteps; inc++) {
      float t = (float)inc / (float)tSteps;
      int deg =
          state == FigureState::SelectVertices ? selectedVrtx : maxVertices - 1;
      Point currPoint = calculateCasteljauPoly(deg, 0, t, vrtxs);
      deployLine(prevPoint, currPoint, lineColor, putPixel);
      prevPoint = currPoint;
    }
    int lastVrtxIdx =
        state == FigureState::SelectVertices ? selectedVrtx : maxVertices - 1;
    deployLine(prevPoint, vrtxs[lastVrtxIdx], lineColor, putPixel);

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
    ImGui::End();
  }
};

int main() {
  proyecto1 app;
  app.run();
  return 0;
}
