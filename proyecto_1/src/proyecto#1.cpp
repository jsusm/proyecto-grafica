#include "engine2D.h"
#include "imgui.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <vector>

enum class FigureType { Line, unknow };

class Figure {
public:
  FigureType type = FigureType::unknow;
  bool selected = false;
  virtual void draw(std::function<void(int, int, const Color &)> putPixel) {};
  virtual void select() {}
  virtual void unselect() {}
  virtual bool onMouseMove(int, int) { return false; }
  virtual bool onMouseClick(int, int) { return false; }
};

typedef struct {
  int x;
  int y;
} Point;

enum class LineState { SelectingStartNode, SelectingEndNode, Done };

void deployLine(Point start, Point end,
                std::function<void(int, int, const Color &)> putPixel) {
  int x, y, dx, dy, d, incE, incNE, _dx, _dy;
  // ajustamos el punto inicial y terminal a conveniencia para usar
  // el caso base (una linea de entre 0 a 45 grados)
  Point _start{0, 0};
  Point _end{end.x - start.x, end.y - start.y};
  // realizamos transformaciones lineales a las coordendas
  // dependiendo en que cuadrante esten
  bool flipx = _end.x < 0;
  bool flipy = _end.y < 0;
  bool flipcoords = std::abs(_end.x) < std::abs(_end.y);
  if (flipx) {
    _end.x = -_end.x;
  }
  if (flipy) {
    _end.y = -_end.y;
  }
  if (flipcoords) {
    int aux = _end.x;
    _end.x = _end.y;
    _end.y = aux;
  }

  dx = _end.x - _start.x;
  dy = _end.y - _start.y;
  d = dx - 2 * dy;
  incE = -dy;
  incNE = dx - dy;
  if (dy < 0) {
    incE = dy;
    incNE = dx + dy;
  }
  x = 0;
  y = 0;

  Color color(1.0f, 1.0f, 1.0f);
  putPixel(x, y, color);

  for (; x < _end.x; x++) {
    if (d <= 0) {
      d += incNE;
      y++;
    } else {
      d += incE;
    }
    int _x = x;
    int _y = y;

    // Desacemos las transformaciones lineales
    // se tienen que deshacer en el orden inverso
    if (flipcoords) {
      int aux = _x;
      _x = _y;
      _y = aux;
    }
    if (flipx) {
      _x = -_x;
    }
    if (flipy) {
      _y = -_y;
    }

    putPixel(start.x + _x, start.y + _y, color);
  }
}

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

enum class ToolsType { Line, Rect, Select };

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
        break;
      case ToolsType::Select:
        break;
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
