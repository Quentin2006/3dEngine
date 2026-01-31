#include "../include/glad/glad.h"
#include "camera.h"
#include "object.h"
#include "shader.h"
#include "window.h"
#include <memory>

struct InputState {
  bool w = false, a = false, s = false, d = false;
  bool q = false, e = false;
  bool up = false, down = false, left = false, right = false;
};

class App {
public:
  App(int width, int height, std::string title);

  void run();
  void move();

  Window window;
  Shader shader;
  Camera camera;
  std::vector<std::unique_ptr<Object>> objs;
  InputState input;

private:
  bool loadShaders();

  unsigned int frameCounter;
};
