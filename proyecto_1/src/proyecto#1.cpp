#include "engine2D.h"
#include "imgui.h"
#include "primitives.h"
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>

enum class LineState { SelectVertices, Selected, DragVertex, Unselected, DragFigure };

class Line : public Figure {
public:
  FigureType type = FigureType::Line;

  std::vector<Point> vrtxs;
  std::vector<bool> vrtxHover{false, false};
  LineState state = LineState::SelectVertices;
  int vrtxRadious = 4;
  int maxVertices = 2;
  int selectedVrtx = 0;
  Color lineColor{1.0f, 1.0f, 1.0f};
  bool hoverCenterVrtx = false;
  Point centerVrtx;
  Color selectedColor = Color(0.1f, 1.0f, 0.1f);
  Point dragOrigin;

  void stateMachine(int branch) {
    switch (state) {
    case LineState::SelectVertices:
      if (vrtxs.size() == maxVertices) {
        state = LineState::Selected;
      }
      break;
    case LineState::DragVertex:
      state = LineState::Selected;
      break;
    case LineState::Selected:
      if (branch == 1) {
        state = LineState::DragVertex;
      }
      else if(branch == 2) {
        state = LineState::DragFigure;
      }
      else {
        state = LineState::Unselected;
      }
      break;
    case LineState::DragFigure:
      state=LineState::Selected;
      break;
    case LineState::Unselected:
      state = LineState::Selected;
      break;
    }
  }

  Line() {
    vrtxs.push_back(Point());
    vrtxs.push_back(Point());
  }

  void draw(std::function<void(int, int, const Color &)> putPixel) {
    // do not show the line if we are selecting the starting node
    if (vrtxs.size() == 0) {
      return;
    }

    deployLine(vrtxs[0], vrtxs[1], lineColor, putPixel);

    // Show control points
    if (state == LineState::Selected) {
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]) {
          deployCircle(vrtxs[i], vrtxRadious, selectedColor, putPixel);
        } else {
          deployCircle(vrtxs[i], vrtxRadious, lineColor, putPixel);
        }
      }
      if (hoverCenterVrtx) {
        deployCircle(centerVrtx, 4, selectedColor, putPixel);
      } else {
        deployCircle(centerVrtx, 4, lineColor, putPixel);
      }
    }
  }

  bool mouseInVrtx(Point vrtx, int x, int y) {
    int dx = vrtx.x - x;
    int dy = vrtx.y - y;
    int distanceSquared = dx * dx + dy * dy;
    return distanceSquared <= vrtxRadious * vrtxRadious;
  }

  void updateVrtxHover(int x, int y) {
    for (int i = 0; i < vrtxs.size(); i++) {
      vrtxHover[i] = mouseInVrtx(vrtxs[i], x, y);
    }
    hoverCenterVrtx = mouseInVrtx(centerVrtx, x, y);
  }

  void onMouseMove(int x, int y) {
    switch (state) {
    case LineState::Selected:
      updateVrtxHover(x, y);
      break;

    case LineState::Unselected:
      break;

    case LineState::SelectVertices:
      vrtxs[selectedVrtx].x = x;
      vrtxs[selectedVrtx].y = y;
      break;

    case LineState::DragVertex:
      vrtxs[selectedVrtx].x = x;
      vrtxs[selectedVrtx].y = y;
      break;

    case LineState::DragFigure:
      updateVrtxHover(x, y);
      int dx = x - dragOrigin.x;
      int dy = y - dragOrigin.y;
      for(int i = 0; i < vrtxs.size(); i++){
        vrtxs[i].x += dx;
        vrtxs[i].y += dy;
      }
      dragOrigin.x = x;
      dragOrigin.y = y;
      break;
    }
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

  void onMouseButtonDown(int x, int y) {
    updateVrtxHover(x, y);
    switch (state) {
    case LineState::SelectVertices:
      for (int i = selectedVrtx; i < vrtxs.size(); i++) {
        vrtxs[i].x = x;
        vrtxs[i].y = y;
      }
      selectedVrtx++;
      if (selectedVrtx == maxVertices) {
        updateCenterPoint();
        stateMachine(0);
      }
      break;
    case LineState::Selected:
      for (int i = 0; i < vrtxs.size(); i++) {
        if (vrtxHover[i]){
          selectedVrtx = i;
          stateMachine(1);
          break;
        }
      }
      if(hoverCenterVrtx) {
        stateMachine(2);
        dragOrigin.x = x;
        dragOrigin.y = y;
        break;
      }
      break;
    case LineState::DragFigure:
      break;
    case LineState::Unselected:
      break;
    }
  }

  void onMouseButtonUp(int x, int y) {
    switch (state) {
    case LineState::Selected:
      break;
    case LineState::DragVertex:
      vrtxs[selectedVrtx].x = x;
      vrtxs[selectedVrtx].y = y;
      updateCenterPoint();
      stateMachine(0);
      break;
    case LineState::DragFigure:
      stateMachine(0);
      updateCenterPoint();
      updateVrtxHover(x, y);
      break;

    case LineState::Unselected:
      break;
    }
  }

  void isMouseOver(int x, int y) {
    int a, b, c;
    // a = vrtx1.y - vrtx2.y;                      // -dy
    // b = vrtx2.x - vrtx1.x;                      // dx
    // c = -vrtx1.y * b - vrtx1.x * a;             // -y0*dx + x0*dy
    // int distance = std::abs(a * x + b * y + c); // x*-dy + y*dx - y0*dx +
    // x0*dy
    //
    // if (distance < 3000) {
    //   lineColor = Color{0.2f, 0.2f, 1.0f};
    // } else {
    //   lineColor = Color{1.0f, 1.0f, 1.0f};
    // }
  }
};

enum class ToolsType { Line, Select };

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
  std::vector<Figure *> figures{};

  // saves which tool the user is using
  ToolsType currentTool = ToolsType::Line;

  // is hover
  bool isHover = false;

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
    if (currentFigure == nullptr) {
      if (currentTool == ToolsType::Line) {
        currentFigure = new Line();
        figures.push_back(currentFigure);
        currentFigure->onMouseButtonDown(x, y);
      }
    } else {
      currentFigure->onMouseButtonDown(x, y);
    }
  }
  void onMouseButtonUp(int button, double x, double y) override {
    if (x >= WindowWidth - ToolsWindowWidth) {
      return;
    }
    if (currentFigure != nullptr) {
      currentFigure->onMouseButtonUp(x, y);
    }
  }

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
    for (Figure *f : figures) {
      f->draw(
          [this](int x, int y, const Color &color) { putPixel(x, y, color); });
    }
  }

  void toolsSection() {
    ImGui::SeparatorText("Herramientas");
    if (ImGui::Button("Line")) {
      currentTool = ToolsType::Line;
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
