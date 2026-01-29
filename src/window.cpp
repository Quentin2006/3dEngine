#include "window.h"
#include <iostream>

extern "C" int gladLoadGLLoader(void *(*proc)(const char *));

Window::Window(int width, int height, std::string title)
    : width(width), height(height) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

  window = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);
  glfwSetWindowSize(window, width, height);

  if (window == NULL) {
    std::cerr << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
  } else {
    glfwMakeContextCurrent(window);
    if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
      std::cerr << "Failed to initialize GLAD" << std::endl;
      glfwTerminate();
    }
    // glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  }
}

// void processInput(GLFWwindow *window) {
//   if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
//     glfwSetWindowShouldClose(window, true);
// }
