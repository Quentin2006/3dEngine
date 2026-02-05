#pragma once

#include "../include/glad/glad.h"
#include "camera.h"
#include "input_manager.h"
#include "resource_manager.h"
#include "shader.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

constexpr float MOVEMENT_SPEED = 15.f;
constexpr float ROTATION_SPEED = 125.f;

struct ObjectConfig {
  struct Mesh {
    std::string path;
    std::string name;
  } mesh{};

  struct Light {
    glm::vec3 color{0, 0, 0};
    float intensity = 1.0f;
  } light{};

  struct Transform {
    glm::vec3 pos{0, 0, 0};
    glm::vec3 rot{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
  } transform{};

  struct SineAnim {
    glm::vec3 axis{1, 0, 0};
    float amplitude = 0.0f;
    float frequency = 1.0f;
    float phase = 0.0f;
  } sineAnim{};
};

class App {
public:
  App(int width, int height, const std::string &title);

  void run();
  void moveCamera(float deltaTime);

  // GLFW callbacks - static methods that forward to instance
  static void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                          int mods);

  Window window;
  Shader shader;
  Camera camera;
  InputManager inputManager;

private:
  bool loadShaders();
  void loadObjectFromConfig(const ObjectConfig &cfg);
  void loadObjectsFromConfig(const std::vector<ObjectConfig> &configs);

  unsigned int frameCounter;
  ResourceManager resourceManager;
};

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
