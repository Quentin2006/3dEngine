#pragma once

#include "../include/glad/glad.h"
#include "camera.h"
#include "object.h"
#include "shader.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

constexpr float MOVEMENT_SPEED = 15.f;
constexpr float ROTATION_SPEED = 125.f;

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
  void move(float deltaTime);

  Window window;
  Shader shader;
  Camera camera;
  std::vector<std::shared_ptr<Object>> objs;
  InputState input;

private:
  bool loadShaders();
  void addObj(std::string objPath, std::string objName,
              glm::vec3 pos = {0, 0, 0}, glm::vec3 rot = {0, 0, 0},
              glm::vec3 scale = {1, 1, 1});

  unsigned int frameCounter;
};

inline void key_callback(GLFWwindow *window, int key, int, int action, int) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (!app)
    return;

  bool pressed = (action == GLFW_PRESS);
  bool released = (action == GLFW_RELEASE);

  if (!pressed && !released)
    return;

  switch (key) {
  case Controls::MOVE_FORWARD:
    app->input.w = pressed;
    return;
  case Controls::MOVE_BACKWARD:
    app->input.s = pressed;
    return;
  case Controls::MOVE_LEFT:
    app->input.a = pressed;
    return;
  case Controls::MOVE_RIGHT:
    app->input.d = pressed;
    return;
  case Controls::MOVE_DOWN:
    app->input.q = pressed;
    return;
  case Controls::MOVE_UP:
    app->input.e = pressed;
    return;
  case Controls::RELOAD_SHADERS:
    if (pressed) {
      std::cerr << "Reloading shaders" << std::endl;
      app->shader.loadShaders();
    }
    return;
  case Controls::ROTATE_PITCH_UP:
    app->input.up = pressed;
    return;
  case Controls::ROTATE_PITCH_DOWN:
    app->input.down = pressed;
    return;
  case Controls::ROTATE_YAW_LEFT:
    app->input.left = pressed;
    return;
  case Controls::ROTATE_YAW_RIGHT:
    app->input.right = pressed;
    return;
  default:
    return;
  }
}

inline void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (!app)
    return;

  // UPDATE WIDTH
  app->window.setWidth(width);
  app->window.setHeight(height);

  // UPDATE PROJECTION MATRIX
  app->camera.updateAspect(width, height);

  glUniformMatrix4fv(app->shader.getUniformLocation("projection"), 1, GL_FALSE,
                     glm::value_ptr(app->camera.getProjectionMatrix()));

  glViewport(0, 0, width, height);
}
