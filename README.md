## Ejecutar el proyecto

### Linux

```bash
cmake -S . -B build/
make -C build
./build/proyecto_1/proyecto1
```

### Windows

Requisitos:

- Visual Studio 2022 con la carga de trabajo "Desktop development with C++".
- CMake disponible en la terminal.
- Git disponible para que CMake pueda descargar dependencias.

Desde una terminal en la raiz del proyecto:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022"
cmake --build build --config Debug
.\build\proyecto_1\Debug\proyecto1.exe
```

Si usas Ninja o el generador por defecto de tu instalacion:

```powershell
cmake -S . -B build
cmake --build build
.\build\proyecto_1\proyecto1.exe
```
