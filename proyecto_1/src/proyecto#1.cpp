#include "engine2D.h"
#include "imgui.h"
#include "primitives.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>

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

class Line : public Figure {
public:
  Line() {
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

    if (state != FigureState::Unselected &&
        state != FigureState::SelectVertices) {
      BoundingBox bb = getBoundingBox();
      deploySquare(bb.vrtx1, bb.vrtx2, boxColor, putPixel);
    }

    deployLine(vrtxs[0], vrtxs[1], lineColor, putPixel);

    // Show control points
    if (state == FigureState::Selected) {
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          deployCircle(vrtxs[i], vrtxRadius, selectedColor, putPixel);
        } else {
          deployCircle(vrtxs[i], vrtxRadius, lineColor, putPixel);
        }
      }
      if (hoverCenterVrtx) {
        deployCircle(centerVrtx, 4, selectedColor, putPixel);
      } else {
        deployCircle(centerVrtx, 4, lineColor, putPixel);
      }
    }
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverLine(vrtxs[0], vrtxs[1], x, y);
  }
};

class Rect : public Figure {
public:
  Rect() {
    type = FigureType::Rect;
    maxVertices = 4;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverLine(vrtxs[0], vrtxs[2], x, y) ||
                isMouseOverLine(vrtxs[0], vrtxs[3], x, y) ||
                isMouseOverLine(vrtxs[1], vrtxs[2], x, y) ||
                isMouseOverLine(vrtxs[1], vrtxs[3], x, y);
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

  bool onMouseButtonDown(int x, int y) {
    bool result = Figure::onMouseButtonDown(x, y);
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

    if (state != FigureState::Unselected &&
        state != FigureState::SelectVertices) {
      BoundingBox bb = getBoundingBox();
      deploySquare(bb.vrtx1, bb.vrtx2, boxColor, putPixel);
    }

    // deploy lines because the figure can be edited in any quadrilateral.
    deployLine(vrtxs[0], vrtxs[3], lineColor, putPixel);
    deployLine(vrtxs[0], vrtxs[2], lineColor, putPixel);
    deployLine(vrtxs[1], vrtxs[3], lineColor, putPixel);
    deployLine(vrtxs[1], vrtxs[2], lineColor, putPixel);

    // Show control points
    if (state == FigureState::Selected) {
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          deployCircle(vrtxs[i], vrtxRadius, selectedColor, putPixel);
        } else {
          deployCircle(vrtxs[i], vrtxRadius, lineColor, putPixel);
        }
      }
      if (hoverCenterVrtx) {
        deployCircle(centerVrtx, 4, selectedColor, putPixel);
      } else {
        deployCircle(centerVrtx, 4, lineColor, putPixel);
      }
    }
  }
};

class Triangle : public Figure {
public:
  Triangle() {
    type = FigureType::Triangle;
    maxVertices = 3;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void isMouseOver(int x, int y) {
    mouseOver = isMouseOverLine(vrtxs[0], vrtxs[1], x, y) ||
                isMouseOverLine(vrtxs[0], vrtxs[2], x, y) ||
                isMouseOverLine(vrtxs[1], vrtxs[2], x, y);
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }

    if (state != FigureState::Unselected &&
        state != FigureState::SelectVertices) {
      BoundingBox bb = getBoundingBox();
      deploySquare(bb.vrtx1, bb.vrtx2, boxColor, putPixel);
    }

    // deploy lines because the figure can be edited in any quadrilateral.
    deployLine(vrtxs[0], vrtxs[2], lineColor, putPixel);
    deployLine(vrtxs[1], vrtxs[0], lineColor, putPixel);
    deployLine(vrtxs[1], vrtxs[2], lineColor, putPixel);

    // Show control points
    if (state == FigureState::Selected) {
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          deployCircle(vrtxs[i], vrtxRadius, selectedColor, putPixel);
        } else {
          deployCircle(vrtxs[i], vrtxRadius, lineColor, putPixel);
        }
      }
      if (hoverCenterVrtx) {
        deployCircle(centerVrtx, 4, selectedColor, putPixel);
      } else {
        deployCircle(centerVrtx, 4, lineColor, putPixel);
      }
    }
  }
};

class Elipse : public Figure {
public:
  Elipse() {
    type = FigureType::Elipse;
    maxVertices = 2;
    vrtxs.resize(maxVertices);
    vrtxHover.resize(maxVertices, false);
  }

  void isMouseOver(int x, int y) {
    int centerX = (vrtxs[0].x + vrtxs[1].x) / 2;
    int centerY = (vrtxs[0].y + vrtxs[1].y) / 2;
    int a = std::abs(vrtxs[0].y - vrtxs[1].y) / 2;
    int b = std::abs(vrtxs[0].x - vrtxs[1].x) / 2;

    signed long long _x = x - centerX;
    signed long long _y = y - centerY;

    int tolerance = 2;

    signed long long ia = a - tolerance; // inner a
    signed long long ib = b - tolerance; // inner b
    signed long long oa = a + tolerance; // outer b
    signed long long ob = b + tolerance; // outer b

    printf("%lld > %lld, %lld < %lld\n", ia*ia*_x*_x + ib*ib*_y*_y, ia*ia*ib*ib, oa*oa*_x*_x + ob*ob*_y*_y, oa*oa*ob*ob);

    bool isOutOfInnerCircle = (signed long long)ia*ia*_x*_x + ib*ib*_y*_y > (signed long long)ia*ia*ib*ib;
    bool isInOuterCircle = (signed long long)oa*oa*_x*_x + ob*ob*_y*_y < (signed long long)oa*oa*ob*ob;
    mouseOver = isOutOfInnerCircle && isInOuterCircle;
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }

    if (state != FigureState::Unselected &&
        state != FigureState::SelectVertices) {
      BoundingBox bb = getBoundingBox();
      deploySquare(bb.vrtx1, bb.vrtx2, boxColor, putPixel);
    }

    // deploy lines because the figure can be edited in any quadrilateral.
    deployEllipse(vrtxs[0], vrtxs[1], lineColor, putPixel);

    // Show control points
    if (state == FigureState::Selected) {
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          deployCircle(vrtxs[i], vrtxRadius, selectedColor, putPixel);
        } else {
          deployCircle(vrtxs[i], vrtxRadius, lineColor, putPixel);
        }
      }
      if (hoverCenterVrtx) {
        deployCircle(centerVrtx, 4, selectedColor, putPixel);
      } else {
        deployCircle(centerVrtx, 4, lineColor, putPixel);
      }
    }
  }
};

enum class ToolsType { Line, Select, Triangle, Elipse, Rect };

int WindowWidth = 1400;
int WindowHeight = 720;
int ToolsWindowWidth = 300;

class proyecto1 : public Engine2D {
private:
  Color colorFondo = Color(0.1f, 0.1f, 0.15f);
  Color colorPincel = Color(1.0f, 0.0f, 0.0f);
  bool dibujando = false;

public:
  proyecto1()
      : Engine2D(1400, 720,
                 "Proyecto #1 - Gestion y Despliegue de Primitivas") {}

  // points to the figure currently focused
  Figure *currentFigure = nullptr;
  // The list of figures
  std::vector<std::unique_ptr<Figure>> figures{};

  // saves which tool the user is using
  ToolsType currentTool = ToolsType::Line;

  // is hover
  bool isHover = false;

  bool mouseReserved = false;

  void setup() override {
    clear(colorFondo);
    std::cout << "Motor inicializado exitosamente." << std::endl;
  }
  // Eventos
  void onkeyDown(int key) override {
    if (key == GLFW_KEY_Q) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE) {
      clear(colorFondo);
    }
  }
  void onMouseButtonDown(int button, double x, double y) override {
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentTool == ToolsType::Line) {

      figures.push_back(std::make_unique<Line>());
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(x, y);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Rect) {

      figures.push_back(std::make_unique<Rect>());
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(x, y);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Triangle) {

      figures.push_back(std::make_unique<Triangle>());
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(x, y);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Elipse){

      figures.push_back(std::make_unique<Elipse>());
      currentFigure = figures.back().get();
      currentFigure->onMouseButtonDown(x, y);
      currentTool = ToolsType::Select;

    } else if (currentTool == ToolsType::Select)

    {

      if (currentFigure == nullptr) {
        for (auto &f : figures) {
          f->onMouseButtonDown(x, y);
          if (f->mouseOver) {
            currentFigure = f.get();
            f->select();
            break;
          }
        }
      } else {
        mouseReserved = currentFigure->onMouseButtonDown(x, y);
        if (currentTool == ToolsType::Select) {
          if (!mouseReserved && !currentFigure->mouseOver) {
            currentFigure->unselect();
            currentFigure = nullptr;
            for (auto &f : figures) {
              f->onMouseButtonDown(x, y);
              if (f->mouseOver) {
                currentFigure = f.get();
                currentFigure->select();
                break;
              }
            }
          }
        }
      }
    }
  }
  void onMouseButtonUp(int button, double x, double y) override {
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentFigure != nullptr) {
      mouseReserved = currentFigure->onMouseButtonUp(x, y);
      if (!mouseReserved) {
        for (auto &f : figures) {
          f->onMouseButtonUp(x, y);
        }
      }
    }
  }

  void onMouseMove(double x, double y) override {
    // do not register the event if the mouse is over the tools window
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentFigure != nullptr) {
      mouseReserved = currentFigure->onMouseMove(x, y);
      if (!mouseReserved) {
        for (auto &f : figures) {
          f->onMouseMove(x, y);
        }
      }
    } else {
      for (auto &f : figures) {
        f->onMouseMove(x, y);
      }
    }
  }

  void update(float deltaTime) override {
    clear(Color(0.1f, 0.1f, 0.1f));
    for (auto &f : figures) {
      f->draw(
          [this](int x, int y, const Color &color) { putPixel(x, y, color); });
    }
  }

  void toolsSection() {
    ImGui::SeparatorText("Herramientas");
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
    ImGui::SameLine();
    if (ImGui::Button("Elipse")) {
      currentTool = ToolsType::Elipse;
      if (currentFigure != nullptr) {
        currentFigure->unselect();
        currentFigure = nullptr;
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("Select")) {
      currentTool = ToolsType::Select;
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
    ImGui::Separator();
    ImGui::End();
  }
};

int main() {
  proyecto1 app;
  app.run();
  return 0;
}
