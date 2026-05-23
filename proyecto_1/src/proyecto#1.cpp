#include "engine2D.h"
#include "imgui.h"
#include "primitives.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>

enum class RectState { SelectingVrtx1, SelectingVrtx2, Done };

class Rect : public Figure {
public:
  Point vrtx1;
  Point vrtx2;
  RectState state;

  void stateMachine(bool goFoward) {
    switch (state) {
    case RectState::SelectingVrtx1:
      state = RectState::SelectingVrtx2;
      break;
    case RectState::SelectingVrtx2:
      state = RectState::Done;
      break;
    case RectState::Done:
      state = RectState::Done;
      break;
    }
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    if (state != RectState::SelectingVrtx1) {
      deployLine(Point(vrtx1.x, vrtx1.y), Point(vrtx2.x, vrtx1.y), putPixel);
      deployLine(Point(vrtx1.x, vrtx1.y), Point(vrtx1.x, vrtx2.y), putPixel);
      deployLine(Point(vrtx1.x, vrtx2.y), Point(vrtx2.x, vrtx2.y), putPixel);
      deployLine(Point(vrtx2.x, vrtx1.y), Point(vrtx2.x, vrtx2.y), putPixel);
    }
  };

  bool onMouseMove(int x, int y) {
    switch (state) {
    case RectState::SelectingVrtx1:
    case RectState::Done:
      break;
    case RectState::SelectingVrtx2:
      vrtx2.x = x;
      vrtx2.y = y;
      break;
    }
    return false;
  }

  bool onMouseClick(int x, int y) {
    switch (state) {
    case RectState::SelectingVrtx1:
      vrtx1.x = x;
      vrtx1.y = y;
      vrtx2.x = x;
      vrtx2.y = y;
      break;
    case RectState::Done:
      break;
    case RectState::SelectingVrtx2:
      vrtx2.x = x;
      vrtx2.y = y;
      break;
    }
    stateMachine(true);
    return state == RectState::Done;
  }
};

enum class LineState { SelectingStartNode, SelectingEndNode, Done };

class Line : public Figure {
public:
  FigureType type = FigureType::Line;

  Point start{512, 300};
  Point end;
  LineState state = LineState::SelectingStartNode;

  void stateMachine(bool goFoward) {
    switch (state) {
    case LineState::SelectingStartNode:
      state = LineState::SelectingEndNode;
      break;
    case LineState::SelectingEndNode:
      state = LineState::Done;
      break;
    case LineState::Done:
      state = LineState::Done;
      break;
    }
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (state == LineState::SelectingStartNode) {
      return;
    }
    deployLine(start, end, putPixel);
  }

  bool onMouseMove(int x, int y) {
    switch (state) {
    case LineState::SelectingStartNode:
    case LineState::Done:
      break;
    case LineState::SelectingEndNode:
      end.x = x;
      end.y = y;
      break;
    }
    return false;
  }

  bool onMouseClick(int x, int y) {
    switch (state) {
    case LineState::SelectingStartNode:
      start.x = x;
      start.y = y;
      end.x = x;
      end.y = y;
      break;
    case LineState::Done:
      break;
    case LineState::SelectingEndNode:
      end.x = x;
      end.y = y;
      break;
    }
    stateMachine(true);
    return state == LineState::Done;
  }
};

enum class ElipceState { SelectingVrtx1, SelectingVrtx2, Done };

class Elipce : public Figure {
public:
  Point vrtx1;
  Point vrtx2;
  ElipceState state;

  void stateMachine(bool goFoward) {
    switch (state) {
    case ElipceState::SelectingVrtx1:
      state = ElipceState::SelectingVrtx2;
      break;
    case ElipceState::SelectingVrtx2:
      state = ElipceState::Done;
      break;
    case ElipceState::Done:
      state = ElipceState::Done;
      break;
    }
  }

  void drawElipcePoints(int x, int y, const Color &c, int centerx, int centery,
                        int flipcoords,
                        std::function<void(int, int, const Color &)> putPixel) {

    if (flipcoords) {
      int aux = x;
      x = y;
      y = aux;
    }
    putPixel(centerx + x, centery + y, c);
    putPixel(centerx + x, centery - y, c);
    putPixel(centerx - x, centery - y, c);
    putPixel(centerx - x, centery + y, c);
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
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
    Color c(1.0f, 1.0f, 1.0f);
    drawElipcePoints(x, y, c, centerx, centery, flipcoords, putPixel);
    while (b * b * 2 * (x + 1) < a * a * (2 * y - 1)) {
      if (d < 0) {
        d += 4 * (b * b * (2 * x + 3));
      } else {
        d += 4 * (b * b * (2 * x + 3) + a * a * (-2 * y + 2));
        y--;
      }
      x++;
      drawElipcePoints(x, y, c, centerx, centery, flipcoords, putPixel);
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
      drawElipcePoints(x, y, c, centerx, centery, flipcoords, putPixel);
    }
  };
  bool onMouseMove(int x, int y) {
    switch (state) {
    case ElipceState::SelectingVrtx1:
    case ElipceState::Done:
      break;
    case ElipceState::SelectingVrtx2:
      vrtx2.x = x;
      vrtx2.y = y;
      break;
    }
    return false;
  }

  bool onMouseClick(int x, int y) {
    switch (state) {
    case ElipceState::SelectingVrtx1:
      vrtx1.x = x;
      vrtx1.y = y;
      vrtx2.x = x;
      vrtx2.y = y;
      break;
    case ElipceState::Done:
      break;
    case ElipceState::SelectingVrtx2:
      vrtx2.x = x;
      vrtx2.y = y;
      break;
    }
    stateMachine(true);
    return state == ElipceState::Done;
  }
};

enum class ToolsType { Line, Rect, Elipce, Select };

int WindowWidth = 1020;
int WindowHeight = 630;
int ToolsWindowWidth = 300;

class proyecto1 : public Engine2D {
private:
  Color colorFondo = Color(0.1f, 0.1f, 0.15f);
  Color colorPincel = Color(1.0f, 0.0f, 0.0f);
  bool dibujando = false;

public:
  proyecto1()
      : Engine2D(1020, 630,
                 "Proyecto #1 - Gestion y Despliegue de Primitivas") {}

  // points to the figure currently focused
  Figure *currentFigure = nullptr;
  // The list of figures
  std::vector<Figure *> Figures{};

  ToolsType currentTool = ToolsType::Line;

  void setup() override {
    clear(colorFondo);
    std::cout << "Motor inicializado exitosamente." << std::endl;
  }
  // Eventos
  void onkeyDown(int key) override {
    if (key == GLFW_KEY_SPACE) {
      clear(colorFondo);
    }
  }
  void onMouseButtonDown(int button, double x, double y) override {
    // if (button == GLFW_MOUSE_BUTTON_LEFT) {
    //     dibujando = true;
    // }
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentFigure != nullptr) {
      bool done = currentFigure->onMouseClick(x, y);
      if (done) {
        currentFigure = nullptr;
      }
    } else {
      switch (currentTool) {
      case ToolsType::Line:
        currentFigure = new Line();
        Figures.push_back(currentFigure);
        currentFigure->onMouseClick(x, y);
        break;
      case ToolsType::Rect:
        currentFigure = new Rect();
        Figures.push_back(currentFigure);
        currentFigure->onMouseClick(x, y);
        break;
      case ToolsType::Select:
        break;
      case ToolsType::Elipce:
        currentFigure = new Elipce();
        Figures.push_back(currentFigure);
        currentFigure->onMouseClick(x, y);
      }
    }
  }
  void onMouseButtonUp(int button, double x, double y) override {
    // if (button == GLFW_MOUSE_BUTTON_LEFT) {
    //   dibujando = false;
    // }
  }
  // Evento de movimiento continuo
  void onMouseMove(double x, double y) override {
    // do not register the event if the mouse is over the tools window
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentFigure != nullptr) {
      currentFigure->onMouseMove(x, y);
    }
  }

  void update(float deltaTime) override {
    clear(Color(0.1f, 0.1f, 0.1f));
    for (Figure *f : Figures) {
      f->draw(
          [this](int x, int y, const Color &color) { putPixel(x, y, color); });
    }
    // Aquí irían cosas que cambian automáticamente con el tiempo
  }

  void toolsSection() {
    ImGui::SeparatorText("Herramientas");
    if (ImGui::Button("Line")) {
      currentTool = ToolsType::Line;
    }
    ImGui::SameLine();
    if (ImGui::Button("Rect")) {
      currentTool = ToolsType::Rect;
    }
    ImGui::SameLine();
    if (ImGui::Button("Elipce")) {
      currentTool = ToolsType::Elipce;
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

    // Begin

    ImGui::Begin("Herramientas", NULL, window_flags);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Separator();
    toolsSection();
    ImGui::Separator();
    float col[3] = {colorPincel.r, colorPincel.g, colorPincel.b};
    if (ImGui::ColorEdit3("Color Pincel", col)) {
      colorPincel.r = col[0];
      colorPincel.g = col[1];
      colorPincel.b = col[2];
    }
    ImGui::Text("Manten click izquierdo para dibujar.");
    ImGui::Text("Presiona ESPACIO para limpiar.");
    ImGui::End();
  }
};

int main() {
  proyecto1 app;
  app.run();
  return 0;
}
