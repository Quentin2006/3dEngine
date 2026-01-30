#include "../include/glad/glad.h"
#include "buffer.h"
#include "camera.h"
#include "shader.h"
#include "window.h"

struct InputState {
  bool w = false, a = false, s = false, d = false;
  bool q = false, e = false;
  bool up = false, down = false, left = false, right = false;
};

class App {
public:
  App(int width, int height, std::string title);
  void run();

  Window window;
  Shader shader;
  Camera camera;
  Buffer buffer;
  InputState input;

private:
  bool loadShaders();

  unsigned int frameCounter;
};
