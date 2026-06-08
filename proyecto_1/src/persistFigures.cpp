#include "persistFigures.h"
#include <algorithm>
#include <fstream>

namespace {
const char *figuresFileName = "figures.txt";

int colorToInt(float value) {
  value = std::clamp(value, 0.0f, 1.0f);
  return static_cast<int>(value * 255.0f + 0.5f);
}

Color intToColor(int r, int g, int b) {
  return Color(static_cast<float>(r) / 255.0f, static_cast<float>(g) / 255.0f,
               static_cast<float>(b) / 255.0f);
}

void writeColor(std::ofstream &file, const Color &color) {
  file << colorToInt(color.r) << ' ' << colorToInt(color.g) << ' '
       << colorToInt(color.b) << '\n';
}
} // namespace

void saveFigures(SerializedFiguresFile serializedFiguresFile) {
  std::ofstream file(figuresFileName);
  if (!file) {
    return;
  }

  file << serializedFiguresFile.figures.size() << '\n';
  for (const auto &figure : serializedFiguresFile.figures) {
    file << static_cast<int>(figure.type) << '\n';
    writeColor(file, figure.lineColor);
    writeColor(file, figure.fillColor);
    file << (figure.filled ? 't' : 'f') << '\n';
    file << figure.points.size() << '\n';

    for (const auto &point : figure.points) {
      file << point.x << ' ' << point.y << '\n';
    }
  }
}

SerializedFiguresFile loadFigures() {
  SerializedFiguresFile serializedFiguresFile{};
  std::ifstream file(figuresFileName);
  if (!file) {
    serializedFiguresFile.nFigures = 0;
    return serializedFiguresFile;
  }

  int nFigures = 0;
  if (!(file >> nFigures)) {
    return serializedFiguresFile;
  }

  serializedFiguresFile.figures.reserve(nFigures);
  for (int i = 0; i < nFigures; i++) {
    SerializedFigure figure{};
    int type = 0;
    int lineR = 0;
    int lineG = 0;
    int lineB = 0;
    int fillR = 0;
    int fillG = 0;
    int fillB = 0;
    char filled = 'f';

    if (!(file >> type)) {
      break;
    }
    if (!(file >> lineR >> lineG >> lineB)) {
      break;
    }
    if (!(file >> fillR >> fillG >> fillB)) {
      break;
    }
    if (!(file >> filled)) {
      break;
    }
    if (!(file >> figure.nPoints)) {
      break;
    }

    figure.type = static_cast<FigureType>(type);
    figure.lineColor = intToColor(lineR, lineG, lineB);
    figure.fillColor = intToColor(fillR, fillG, fillB);
    figure.filled = filled == 't';
    figure.points.reserve(figure.nPoints);

    for (int j = 0; j < figure.nPoints; j++) {
      Point point{};
      if (!(file >> point.x >> point.y)) {
        return serializedFiguresFile;
      }
      figure.points.push_back(point);
    }

    serializedFiguresFile.figures.push_back(figure);
  }

  serializedFiguresFile.nFigures = serializedFiguresFile.figures.size();
  return serializedFiguresFile;
}
