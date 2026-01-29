# OpenGL Template

OpenGL 3.3 Core template using C++, GLFW, and GLAD.

## Prerequisites

Install GLFW

## Building

```bash
make
```

## Running

```bash
make run
```

## Cleaning

```bash
make clean
```

## Project Structure

- `src/main.cpp` - Application entry point
- `src/app.cpp` / `src/app.h` - Main application class
- `src/window.cpp` / `src/window.h` - GLFW window management
- `src/shader.cpp` / `src/shader.h` - Shader compilation and management
- `src/buffer.cpp` / `src/buffer.h` - OpenGL buffer utilities
- `src/shaders/` - GLSL shader files
  - `vertex.glsl` - Vertex shader
  - `fragment.glsl` - Fragment shader
- `src/glad.c` - GLAD loader (OpenGL 3.3 Core)
- `include/glad/glad.h` - GLAD header
- `include/KHR/khrplatform.h` - KHR platform header
- `Makefile` - Build configuration
- `bin/` - Compiled executable output
- `obj/` - Object files (build artifacts)
