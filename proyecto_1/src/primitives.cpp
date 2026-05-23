#include <functional>
#include "primitives.h"

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
