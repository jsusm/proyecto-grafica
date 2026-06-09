#include "persistFigures.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>

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

std::vector<int> parseInts(const std::string &line) {
  std::istringstream stream(line);
  std::vector<int> values;
  int value = 0;
  while (stream >> value) {
    values.push_back(value);
  }
  return values;
}
} // namespace

void saveFigures(SerializedFiguresFile serializedFiguresFile) {
  std::ofstream file(figuresFileName);
  if (!file) {
    return;
  }

  file << serializedFiguresFile.figures.size() << '\n';
  writeColor(file, serializedFiguresFile.backgroundColor);
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

  std::string line;
  if (!std::getline(file, line)) {
    return serializedFiguresFile;
  }
  std::vector<int> firstLine = parseInts(line);
  if (firstLine.size() != 1) {
    return serializedFiguresFile;
  }
  int nFigures = firstLine[0];

  serializedFiguresFile.figures.reserve(nFigures);
  bool hasBufferedFigureType = false;
  int bufferedFigureType = 0;

  if (std::getline(file, line)) {
    std::vector<int> values = parseInts(line);
    if (values.size() == 3) {
      serializedFiguresFile.backgroundColor =
          intToColor(values[0], values[1], values[2]);
    } else if (values.size() == 1) {
      hasBufferedFigureType = true;
      bufferedFigureType = values[0];
    } else if (nFigures > 0) {
      return serializedFiguresFile;
    }
  }

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

    if (hasBufferedFigureType) {
      type = bufferedFigureType;
      hasBufferedFigureType = false;
    } else {
      if (!std::getline(file, line)) {
        break;
      }
      std::vector<int> values = parseInts(line);
      if (values.size() != 1) {
        break;
      }
      type = values[0];
    }

    if (!std::getline(file, line)) {
      break;
    }
    std::vector<int> lineColor = parseInts(line);
    if (lineColor.size() != 3) {
      break;
    }
    lineR = lineColor[0];
    lineG = lineColor[1];
    lineB = lineColor[2];

    if (!std::getline(file, line)) {
      break;
    }
    std::vector<int> fillColor = parseInts(line);
    if (fillColor.size() != 3) {
      break;
    }
    fillR = fillColor[0];
    fillG = fillColor[1];
    fillB = fillColor[2];

    if (!std::getline(file, line) || line.empty()) {
      break;
    }
    filled = line[0];

    if (!std::getline(file, line)) {
      break;
    }
    std::vector<int> nPoints = parseInts(line);
    if (nPoints.size() != 1) {
      break;
    }
    figure.nPoints = nPoints[0];

    figure.type = static_cast<FigureType>(type);
    figure.lineColor = intToColor(lineR, lineG, lineB);
    figure.fillColor = intToColor(fillR, fillG, fillB);
    figure.filled = filled == 't';
    figure.points.reserve(figure.nPoints);

    for (int j = 0; j < figure.nPoints; j++) {
      Point point{};
      if (!std::getline(file, line)) {
        return serializedFiguresFile;
      }
      std::vector<int> pointValues = parseInts(line);
      if (pointValues.size() != 2) {
        return serializedFiguresFile;
      }
      point.x = pointValues[0];
      point.y = pointValues[1];
      figure.points.push_back(point);
    }

    serializedFiguresFile.figures.push_back(figure);
  }

  serializedFiguresFile.nFigures = serializedFiguresFile.figures.size();
  return serializedFiguresFile;
}
