#pragma once

#include "primitives.h"
#include <vector>

struct SerializedFigure {
  FigureType type;
  Color lineColor;
  Color fillColor;
  bool filled;
  int nPoints;
  std::vector<Point> points;
};
struct SerializedFiguresFile {
  int nFigures;
  std::vector<SerializedFigure> figures;
};

void saveFigures(SerializedFiguresFile);

SerializedFiguresFile loadFigures();
