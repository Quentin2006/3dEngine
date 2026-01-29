#include "../include/glad/glad.h"
#include "buffer.h"
#include "shader.h"
#include "window.h"

class App {
public:
  App(int width, int height, std::string title);
  void run();

private:
  bool loadShaders();

  Window window;
  Shader shader;
  Buffer buffer;
  unsigned int frameCounter;
};
