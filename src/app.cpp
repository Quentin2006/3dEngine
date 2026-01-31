#include "app.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

constexpr float MOVEMENT_SPEED = 0.05f;
constexpr float ROTATION_SPEED = 0.7f;
const std::vector<glm::vec3> cube = {
    // Back face - viewed from -Z, CCW winding
    // Normal: (0, 0, -1)
    glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
    glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
    glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),

    // Front face - viewed from +Z, CCW winding
    // Normal: (0, 0, 1)
    glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),

    // Left face - viewed from -X, CCW winding
    // Normal: (-1, 0, 0)
    glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
    glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),
    glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),

    // Right face - viewed from +X, CCW winding
    // Normal: (1, 0, 0)
    glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
    glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),

    // Bottom face - viewed from -Y, CCW winding
    // Normal: (0, -1, 0)
    glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
    glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),
    glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),

    // Top face - viewed from +Y, CCW winding
    // Normal: (0, 1, 0)
    glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, 0.5f, -0.5f)};
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
App::App(int width, int height, std::string title)
    : window(width, height, title), shader(), camera(45.f, width, height),
      frameCounter(0) {
  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    // Handle error (throw exception or exit)
    exit(-1);
  }

  // load the shaders
  shader.loadShaders();

  // CONFIG
  glViewport(0, 0, window.getWidth(), window.getHeight());
  glClearColor(0.05f, 0.1f, 0.1f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  // Face culling - skip rendering inside faces
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Blending - enables transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // needed to get class instance in window callback
  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  glfwSetKeyCallback(window.getGLFWwindow(), key_callback);
}

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  unsigned int modelLoc = shader.addUBO("model");
  unsigned int viewLoc = shader.addUBO("view");
  unsigned int projLoc = shader.addUBO("projection");
  unsigned int timeLoc = shader.addUBO("time");

  // Test 1: Position - 5 cubes in a row at different X positions
  for (int i = 0; i < 5; i++) {
    auto obj = std::make_unique<Object>();
    obj->loadVertices(cube);
    obj->setPosition({i * 2.0f - 4.0f, 0, 0}); // -4, -2, 0, 2, 4
    objs.push_back(std::move(obj));
  }

  // Test 2: Scale - one big cube at top
  auto bigCube = std::make_unique<Object>();
  bigCube->loadVertices(cube);
  bigCube->setPosition({0, 3, 0});
  bigCube->setScale({2, 2, 2}); // 2x bigger
  objs.push_back(std::move(bigCube));

  // Test 3: Rotation - 4 rotated cubes at bottom
  for (int i = 0; i < 4; i++) {
    auto obj = std::make_unique<Object>();
    obj->loadVertices(cube);
    obj->setPosition({(i - 1.5f) * 2.0f, -3, 0});
    obj->setRotation({0, i * 45.0f, 0}); // 0°, 45°, 90°, 135°
    objs.push_back(std::move(obj));
  }

  // Test 4: Combined transform - small rotated cube
  auto combined = std::make_unique<Object>();
  combined->loadVertices(cube);
  combined->setPosition({3, 3, 0});
  combined->setRotation({45, 45, 0});
  combined->setScale({0.5, 0.5, 0.5}); // Half size
  objs.push_back(std::move(combined));

  while (!window.shouldClose()) {
    move();

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update time
    glUniform1f(timeLoc, (float)glfwGetTime());

    // 1. projection
    //  Only needs to change if window resizes, but fine to set every frame for
    // now
    glUniformMatrix4fv(projLoc, 1, GL_FALSE,
                       glm::value_ptr(camera.getProjectionMatrix()));

    // 2. view
    // position of camersa
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE,
                       glm::value_ptr(camera.getViewMatrix()));

    // 3. model
    // what encode the scale, position, and rotation
    for (auto &obj : objs) {
      obj->updateModelMatrix();
      glUniformMatrix4fv(modelLoc, 1, GL_FALSE,
                         glm::value_ptr(obj->getModelMatrix()));
      obj->draw();
    }

    window.swapBuffers();
  }

  glfwTerminate();
}

// would like to abstract into camera class
void App::move() {
  if (input.w)
    camera.moveForward(MOVEMENT_SPEED);
  if (input.s)
    camera.moveForward(-MOVEMENT_SPEED);
  if (input.a)
    camera.moveRight(-MOVEMENT_SPEED);
  if (input.d)
    camera.moveRight(MOVEMENT_SPEED);
  if (input.q)
    camera.moveUp(MOVEMENT_SPEED);
  if (input.e)
    camera.moveUp(-MOVEMENT_SPEED);
  if (input.up)
    camera.rotatePitch(ROTATION_SPEED);
  if (input.down)
    camera.rotatePitch(-ROTATION_SPEED);
  if (input.left)
    camera.rotateYaw(-ROTATION_SPEED);
  if (input.right)
    camera.rotateYaw(ROTATION_SPEED);
}
