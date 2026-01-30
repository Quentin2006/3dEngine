#include "app.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <vector>

constexpr float MOVEMENT_SPEED = 0.05f;
constexpr float ROTATION_SPEED = 0.7f;

const std::vector<glm::vec3> cube = {
    // Back face
    glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
    glm::vec3(0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f),
    glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),

    // Front face
    glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, 0.5f),

    // Left face
    glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f),
    glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),
    glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, 0.5f),

    // Right face
    glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, -0.5f),
    glm::vec3(0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
    glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),

    // Bottom face
    glm::vec3(-0.5f, -0.5f, -0.5f), glm::vec3(0.5f, -0.5f, -0.5f),
    glm::vec3(0.5f, -0.5f, 0.5f), glm::vec3(0.5f, -0.5f, 0.5f),
    glm::vec3(-0.5f, -0.5f, 0.5f), glm::vec3(-0.5f, -0.5f, -0.5f),

    // Top face
    glm::vec3(-0.5f, 0.5f, -0.5f), glm::vec3(0.5f, 0.5f, -0.5f),
    glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(-0.5f, 0.5f, 0.5f), glm::vec3(-0.5f, 0.5f, -0.5f)};

void key_callback(GLFWwindow *window, int key, int, int action, int) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (!app)
    return;

  bool pressed = (action == GLFW_PRESS);
  bool released = (action == GLFW_RELEASE);

  if (!pressed && !released)
    return;

  switch (key) {
  case GLFW_KEY_W:
    app->input.w = pressed;
    return;
  case GLFW_KEY_S:
    app->input.s = pressed;
    return;
  case GLFW_KEY_A:
    app->input.a = pressed;
    return;
  case GLFW_KEY_D:
    app->input.d = pressed;
    return;
  case GLFW_KEY_Q:
    app->input.q = pressed;
    return;
  case GLFW_KEY_E:
    app->input.e = pressed;
    return;
  case GLFW_KEY_R:
    if (pressed) {
      std::cerr << "Reloading shaders" << std::endl;
      app->shader.loadShaders();
    }
    return;
  case GLFW_KEY_UP:
    app->input.up = pressed;
    return;
  case GLFW_KEY_DOWN:
    app->input.down = pressed;
    return;
  case GLFW_KEY_LEFT:
    app->input.left = pressed;
    return;
  case GLFW_KEY_RIGHT:
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
  glEnable(GL_DEPTH_TEST);

  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  glfwSetKeyCallback(window.getGLFWwindow(), key_callback);
}

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  unsigned int modelLoc =
      glGetUniformLocation(shader.getShaderProgram(), "model");
  unsigned int viewLoc =
      glGetUniformLocation(shader.getShaderProgram(), "view");
  unsigned int projLoc =
      glGetUniformLocation(shader.getShaderProgram(), "projection");

  // load the vertices into the buffer and bind the arrat to VAO
  buffer.loadVertices(cube);

  while (!window.shouldClose()) {
    // Process input for smooth movement
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

    // clear the screen
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // activate shader
    glUseProgram(shader.getShaderProgram());

    // 1. projection
    // Only needs to change if window resizes, but fine to set every frame for
    // now
    glUniformMatrix4fv(projLoc, 1, GL_FALSE,
                       glm::value_ptr(camera.getProjectionMatrix()));

    // 2. view
    // position of camersa
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE,
                       glm::value_ptr(camera.getViewMatrix()));

    // 3. model
    // what encode the scale, position, and rotation
    glm::mat4 model = glm::mat4(1.0f);
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    // bind vao
    glBindVertexArray(buffer.getVAO());

    // draw
    glDrawArrays(GL_TRIANGLES, 0, cube.size());

    // snap and pull
    window.swapBuffers();
    glfwPollEvents();
  }

  glfwTerminate();
}
