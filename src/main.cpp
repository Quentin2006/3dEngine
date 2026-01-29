#include "app.h"

const unsigned int WIDTH = 500;
const unsigned int HEIGHT = 500;

int main() {
  App app(WIDTH, HEIGHT, "OpenGL Template");

  app.run();

  return 0;
}
