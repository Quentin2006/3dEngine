#include "app.h"
#include "ecs/systems.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

using std::string;

extern Registry g_registry;

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
  glClearColor(0, 0, .1f, 1.f);
  glEnable(GL_DEPTH_TEST);

  // Face culling - skip rendering inside faces
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // needed to get class instance in window callback
  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  glfwSetKeyCallback(window.getGLFWwindow(), key_callback);
  glfwSetFramebufferSizeCallback(window.getGLFWwindow(),
                                 framebuffer_size_callback);
}

void App::addObj(std::string objPath, string objName, glm::vec3 pos,
                 glm::vec3 rot, glm::vec3 scale) {
  auto obj = std::make_shared<Object>();
  std::cerr << "Loading: " << obj->loadMesh(objPath, objName) << " verts"
            << std::endl;
  obj->setScale(scale);
  obj->setPosition(pos);
  obj->setRotation(rot);
  objs.push_back(obj);
}

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  shader.addUniform("model");
  shader.addUniform("view");
  shader.addUniform("projection");
  shader.addUniform("ourTexture");
  shader.addUniform("lightPos");
  shader.addUniform("lightColor");

  std::string CUBE_PATH = "assets/3d-cubes/";
  std::string HUMAN_PATH = "assets/human/";
  std::string CAR_PATH = "assets/Car/";
  std::string WOLF_PATH = "assets/wolf/";

  addObj(HUMAN_PATH, "FinalBaseMesh.obj");
  addObj(CAR_PATH, "Car.obj", {10, 0, 10}, {0, 0, 0}, {.1, .1, .1});
  addObj(CUBE_PATH, "cube.obj");
  addObj(WOLF_PATH, "Wolf_obj.obj", {10, 5, 1}, {0, 0, 0}, {2, 2, 2});

  auto prevTime = std::chrono::steady_clock::now();
  float fpsTimer;
  float totalTime;

  while (!window.shouldClose()) {
    auto currentTime = std::chrono::steady_clock::now();
    auto deltaTime =
        std::chrono::duration<float>(currentTime - prevTime).count();
    prevTime = currentTime;

    totalTime += deltaTime;
    fpsTimer += deltaTime;

    if (fpsTimer >= 1) {
      std::cerr << "\rfps: " << 1 / deltaTime << std::flush;
      fpsTimer = 0;
    }

    // Set texture unit for all objects
    glActiveTexture(GL_TEXTURE0);
    glUniform1i(shader.getUniformLocation("ourTexture"), 0);

    // Find light position and color from ECS
    glm::vec3 lightPos;
    glm::vec3 lightColor(1, 1, 1);
    for (size_t i = 0; i < g_registry.entityCount(); i++) {
      auto &lightOpt = g_registry.getLight(i);
      if (lightOpt.has_value()) {
        // Update light position
        auto &transform = g_registry.getTransform(i);
        transform.position = {glm::sin(totalTime) * 5.0f, 0.0f, 0.0f};
        lightPos = transform.position;
        lightColor = lightOpt->color;
        break;
      }
    }
    glUniform3f(shader.getUniformLocation("lightPos"), lightPos.x, lightPos.y,
                lightPos.z);
    glUniform3f(shader.getUniformLocation("lightColor"), lightColor.r,
                lightColor.g, lightColor.b);

    // moves camera object
    move(deltaTime);

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update camera
    glUniformMatrix4fv(shader.getUniformLocation("view"), 1, GL_FALSE,
                       glm::value_ptr(camera.getViewMatrix()));

    // Update and render via ECS systems
    updateTransforms(g_registry);
    renderAll(g_registry, shader.getUniformLocation("model"));

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
