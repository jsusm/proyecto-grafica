#include "engine2D.h"
#include "figures.h"
#include "imgui.h"
#include "persistFigures.h"
#include "primitives.h"
#include "quadTree.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <unordered_set>
#include <vector>

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
  bool showQuadTree = false;
  QuadTree quadTree{QuadTreeRect{0, 0, WindowWidth - ToolsWindowWidth,
                                 WindowHeight}};

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

  void rebuildQuadTree() {
    quadTree.clear();
    for (auto &figure : figures) {
      quadTree.insert(figure.get());
    }
  }

  std::vector<Figure *> getMouseCandidateFigures(int x, int y) {
    rebuildQuadTree();
    return quadTree.queryPoint(x, y);
  }

  std::unordered_set<Figure *> getMouseCandidateSet(int x, int y) {
    std::vector<Figure *> candidates = getMouseCandidateFigures(x, y);
    return std::unordered_set<Figure *>(candidates.begin(), candidates.end());
  }

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
      currentFigure->setState(FigureState::Unselected);
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

  bool elevateCurrentFigure() {
    if (currentFigure == nullptr || figures.size() < 2) {
      return false;
    }

    for (size_t i = 0; i + 1 < figures.size(); i++) {
      if (figures[i].get() == currentFigure) {
        std::swap(figures[i], figures[i + 1]);
        return true;
      }
    }

    return false;
  }

  bool sinkCurrentFigure() {
    if (currentFigure == nullptr || figures.size() < 2) {
      return false;
    }

    for (size_t i = 1; i < figures.size(); i++) {
      if (figures[i].get() == currentFigure) {
        std::swap(figures[i], figures[i - 1]);
        return true;
      }
    }

    return false;
  }

  bool deleteCurrentFigure() {
    if (currentFigure == nullptr) {
      return false;
    }

    for (auto it = figures.begin(); it != figures.end(); ++it) {
      if (it->get() == currentFigure) {
        figures.erase(it);
        currentFigure = nullptr;
        mouseReserved = false;
        isHover = false;
        return true;
      }
    }

    currentFigure = nullptr;
    return false;
  }

  // Eventos
  void onkeyDown(int key) override {
    if (key == GLFW_KEY_Q) {
      glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (key == GLFW_KEY_SPACE) {
      clear(backgroundColor);
    }
    if (key == GLFW_KEY_BACKSPACE) {
      deleteCurrentFigure();
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
        std::unordered_set<Figure *> candidates = getMouseCandidateSet(x, y);
        for (auto it = figures.rbegin(); it != figures.rend(); ++it) {
          if (candidates.find(it->get()) == candidates.end()) {
            continue;
          }
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
            std::unordered_set<Figure *> candidates = getMouseCandidateSet(x, y);
            for (auto it = figures.rbegin(); it != figures.rend(); ++it) {
              if (candidates.find(it->get()) == candidates.end()) {
                continue;
              }
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
        std::vector<Figure *> candidates = getMouseCandidateFigures(x, y);
        for (Figure *figure : candidates) {
          figure->onMouseButtonUp(button, x, y);
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
        std::vector<Figure *> candidates = getMouseCandidateFigures(x, y);
        for (Figure *figure : candidates) {
          figure->onMouseMove(x, y);
          isHover |= figure->mouseOver;
        }
      }
    } else {
      std::vector<Figure *> candidates = getMouseCandidateFigures(x, y);
      for (Figure *figure : candidates) {
        figure->onMouseMove(x, y);
        isHover |= figure->mouseOver;
      }
    }
  }

  void update(float deltaTime) override {
    rebuildQuadTree();
    clear(backgroundColor);
    for (auto &f : figures) {
      f->draw(
          [this](int x, int y, const Color &color) { putPixel(x, y, color); });
    }
    if (showQuadTree) {
      quadTree.drawLeaves(
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
      if (currentFigure != nullptr) {
        currentFigure->unselect();
        currentFigure = nullptr;
      }
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
    ImGui::Checkbox("Mostrar QuadTree", &showQuadTree);
    if (ImGui::Button("Guardar en disco")) {
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
      if (ImGui::Button("Delete Figure")) {
        deleteCurrentFigure();
      } else {
        if (ImGui::Button("Move Figure to Foward")) {
          elevateCurrentFigure();
        }
        if (ImGui::Button("Move Figure to Backward")) {
          sinkCurrentFigure();
        }
        if (currentFigure->type == FigureType::BezierCurve) {
          if (ImGui::Button("Elevate Bezier Curve Grade")) {
            BezierCurve *bc = static_cast<BezierCurve *>(currentFigure);
            bc->elevateGrade();
          }
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
