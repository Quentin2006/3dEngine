# OpenGL Template

OpenGL 3.3 Core template using C++, GLFW, and GLAD.

## Prerequisites

Install GLFW development library:
```bash
sudo apt-get install libglfw3-dev
```

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

- `src/main.cpp` - Main application code
- `src/shaders/` - GLSL shader files
- `src/glad.c` - GLAD loader (OpenGL 3.3 Core)
- `include/glad/glad.h` - GLAD header
- `include/KHR/khrplatform.h` - KHR platform header
- `Makefile` - Build configuration
