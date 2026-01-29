#include <GLFW/glfw3.h>
#include <string>
class Window {
public:
  Window(int width, int height, std::string title);

  int getWidth() { return width; }
  int getHeight() { return height; }

  bool shouldClose() { return glfwWindowShouldClose(window); }

  void swapBuffers() { glfwSwapBuffers(window); }

  void setWindowCallback(GLFWkeyfun callback) {
    glfwSetKeyCallback(window, callback);
  }

private:
  GLFWwindow *window;
  int width, height;
};
