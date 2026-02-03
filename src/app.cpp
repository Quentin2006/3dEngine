#include "app.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

constexpr float MOVEMENT_SPEED = 15.f;
constexpr float ROTATION_SPEED = 125.f;

void key_callback(GLFWwindow *window, int key, int, int action, int) {
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

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
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

App::App(int width, int height, std::string title)
    : window(width, height, title), shader(),
      camera(45.f, width, height, 0.1f, 1000.f), frameCounter(0) {
  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    // Handle error (throw exception or exit)
    exit(-1);
  }

  // load the shaders
  shader.loadShaders();

  // CONFIG
  glViewport(0, 0, window.getWidth(), window.getHeight());
  glClearColor(160 / 255.f, 217 / 255.f, 239 / 255.f, 1.f);
  glEnable(GL_DEPTH_TEST);

  // Face culling - skip rendering inside faces
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Blending - enables transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Textures
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);

  // needed to get class instance in window callback
  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  glfwSetKeyCallback(window.getGLFWwindow(), key_callback);
  glfwSetFramebufferSizeCallback(window.getGLFWwindow(),
                                 framebuffer_size_callback);
}

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  shader.addUniform("model");
  shader.addUniform("view");
  shader.addUniform("projection");
  shader.addUniform("time");
  shader.addUniform("ourTexture");

  auto obj = std::make_unique<Object>();
  std::string OBJ_PATH = "assets/911_scene/911_scene.obj";
  std::string TEXTURE_PATH = "assets/Car/Car.png";
  std::cerr << "Loaded " << obj->loadObj(OBJ_PATH) << "verts" << std::endl;
  // obj->setTexture(TEXTURE_PATH);
  objs.push_back(std::move(obj));

  auto prevTime = std::chrono::steady_clock::now();
  float totalTime;

  while (!window.shouldClose()) {
    auto currentTime = std::chrono::steady_clock::now();
    auto deltaTime =
        std::chrono::duration<float>(currentTime - prevTime).count();
    prevTime = currentTime;

    totalTime += deltaTime;

    // moves camera object
    move(deltaTime);

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update time
    glUniform1f(shader.getUniformLocation("time"), (float)glfwGetTime());

    // 1. view
    // position of camera
    glUniformMatrix4fv(shader.getUniformLocation("view"), 1, GL_FALSE,
                       glm::value_ptr(camera.getViewMatrix()));

    // Set texture unit for all objects
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(shader.getUniformLocation("ourTexture"), 0);

    // 2. model
    // what encode the scale, position, and rotation
    for (auto &obj : objs) {
      obj->updateModelMatrix();
      glUniformMatrix4fv(shader.getUniformLocation("model"), 1, GL_FALSE,
                         glm::value_ptr(obj->getModelMatrix()));
      obj->draw();
    }

    window.swapBuffers();
  }

  glfwTerminate();
}

// would like to abstract into camera class
void App::move(float deltaTime) {
  if (input.w)
    camera.moveForward(MOVEMENT_SPEED * deltaTime);
  if (input.s)
    camera.moveForward(-MOVEMENT_SPEED * deltaTime);
  if (input.a)
    camera.moveRight(-MOVEMENT_SPEED * deltaTime);
  if (input.d)
    camera.moveRight(MOVEMENT_SPEED * deltaTime);
  if (input.q)
    camera.moveUp(MOVEMENT_SPEED * deltaTime);
  if (input.e)
    camera.moveUp(-MOVEMENT_SPEED * deltaTime);
  if (input.up)
    camera.rotatePitch(ROTATION_SPEED * deltaTime);
  if (input.down)
    camera.rotatePitch(-ROTATION_SPEED * deltaTime);
  if (input.left)
    camera.rotateYaw(-ROTATION_SPEED * deltaTime);
  if (input.right)
    camera.rotateYaw(ROTATION_SPEED * deltaTime);
}
