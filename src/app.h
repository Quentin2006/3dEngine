#include "../include/glad/glad.h"
#include "camera.h"
#include "object.h"
#include "shader.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <memory>

struct InputState {
  bool w = false, a = false, s = false, d = false;
  bool q = false, e = false;
  bool up = false, down = false, left = false, right = false;
};

struct Controls {
  // Camera movement
  static const int MOVE_FORWARD = GLFW_KEY_W;  // Move camera forward
  static const int MOVE_BACKWARD = GLFW_KEY_S; // Move camera backward
  static const int MOVE_LEFT = GLFW_KEY_A;     // Move camera left (strafe)
  static const int MOVE_RIGHT = GLFW_KEY_D;    // Move camera right (strafe)
  static const int MOVE_UP = GLFW_KEY_E;       // Move camera up
  static const int MOVE_DOWN = GLFW_KEY_Q;     // Move camera down

  // Camera rotation
  static const int ROTATE_PITCH_UP = GLFW_KEY_UP; // Look up (rotate around X)
  static const int ROTATE_PITCH_DOWN =
      GLFW_KEY_DOWN; // Look down (rotate around X)
  static const int ROTATE_YAW_LEFT =
      GLFW_KEY_LEFT; // Look left (rotate around Y)
  static const int ROTATE_YAW_RIGHT =
      GLFW_KEY_RIGHT; // Look right (rotate around Y)

  // Actions
  static const int RELOAD_SHADERS = GLFW_KEY_R; // Hot-reload shader files
};

class App {
public:
  App(int width, int height, std::string title);

  void run();
  void move();

  unsigned int projLoc;

  Window window;
  Shader shader;
  Camera camera;
  std::vector<std::unique_ptr<Object>> objs;
  InputState input;

private:
  bool loadShaders();

  unsigned int frameCounter;
};
