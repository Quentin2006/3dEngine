#include "app.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

void key_callback(GLFWwindow *window, int key, int scancode, int action,
                  int mods) {
  if (key == GLFW_KEY_W) {
  }
}

App::App(int width, int height, std::string title)
    : window(width, height, title), shader(), frameCounter(0) {
  shader.loadShaders();
}

void App::run() {
  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    return;
  }

  glViewport(0, 0, window.getWidth(), window.getHeight());

  std::vector<float> verticies({-0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f, 0.0f});
  while (!window.shouldClose()) {
    buffer.loadVertices(verticies);

    window.setWindowCallback(key_callback);

    // will check for shader changes every 60 frames
    frameCounter++;
    if (frameCounter >= 60) {
      shader.loadShaders();
      frameCounter = 0;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(shader.getShaderProgram());
    glBindVertexArray(buffer.getVAO());
    glDrawArrays(GL_TRIANGLES, 0, 3);

    window.swapBuffers();
    glfwPollEvents();
  }
  glfwTerminate();

  return;
}
