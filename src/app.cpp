#include "app.h"
#include "ecs/systems.h"
#include "resource_manager.h"
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

void fps(float deltaTime) {
  static float fpsTimer = 0;
  fpsTimer += deltaTime;

  if (fpsTimer >= 1) {
    std::cerr << "\rfps: " << 1 / deltaTime << std::flush;
    fpsTimer = 0;
  }
}

App::App(int width, int height, const std::string &title)
    : window(width, height, title), shader(),
      camera(45.f, width, height, 0.1f, 1000.f), frameCounter(0) {
  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
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

void App::loadObjectFromConfig(const ObjectConfig &cfg) {
  int obj = registry.createEntity();
  registry.getMesh(obj).mesh =
      resourceManager.loadMesh(cfg.mesh.path, cfg.mesh.name);
  registry.getTransform(obj) = cfg.transform;

  if (cfg.light.color != glm::vec3(0, 0, 0)) {
    registry.getLight(obj) = Light{cfg.light.color, cfg.light.intensity};
  }

  if (cfg.sineAnim.amplitude != 0.f) {
    registry.getSineAnimator(obj) = {
        cfg.sineAnim.axis,
        cfg.sineAnim.amplitude,
        cfg.sineAnim.frequency,
        cfg.sineAnim.phase,
    };
  }
}

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  shader.addUniform("model");
  shader.addUniform("view");
  shader.addUniform("projection");
  shader.addUniform("ourTexture");
  shader.addUniform("lightPos");
  shader.addUniform("lightColor");

  const std::vector<ObjectConfig> objectConfigs = {
      {.mesh = {"assets/human/", "FinalBaseMesh.obj"}},

      {.mesh = {"assets/Car/", "Car.obj"},
       .light = {{0, 0, 0}, 1.0f},
       .transform = {{10, 0, 10}, {0, 0, 0}, {.1, .1, .1}}},

      {.mesh = {"assets/3d-cubes/", "cube-tex.obj"},
       .light = {{1, 1, 1}, 1.0f},
       .transform = {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}},
       .sineAnim = {{1, 0, 0}, .01, 1.5, 1}},

      {.mesh = {"assets/wolf/", "Wolf_obj.obj"},
       .light = {{0, 0, 0}, 1.0f},
       .transform = {{10, 5, 1}, {0, 0, 0}, {2, 2, 2}}},
  };

  for (const auto &cfg : objectConfigs) {
    loadObjectFromConfig(cfg);
  }

  auto prevTime = std::chrono::steady_clock::now();
  float totalTime = 0;

  while (!window.shouldClose()) {
    auto currentTime = std::chrono::steady_clock::now();
    auto deltaTime =
        std::chrono::duration<float>(currentTime - prevTime).count();
    prevTime = currentTime;
    totalTime += deltaTime;
    fps(deltaTime);

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update camera
    moveCamera(deltaTime);
    glUniformMatrix4fv(shader.getUniformLocation("view"), 1, GL_FALSE,
                       glm::value_ptr(camera.getViewMatrix()));

    // Update and render via ECS systems
    updateTransforms(registry);
    updateAnimations(registry, totalTime);

    renderAll(registry, shader.getUniformLocation("model"),
              shader.getUniformLocation("lightPos"),
              shader.getUniformLocation("lightColor"));

    window.swapBuffers();
  }

  glfwTerminate();
}

// would like to abstract into camera class
void App::moveCamera(float deltaTime) {
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
