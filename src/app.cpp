#include "app.h"
#include "ecs/components.h"
#include "ecs/systems.h"
#include "resource_manager.h"
#include "uniformBuffer.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <glm/ext/vector_float3.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ostream>

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
      camera(45.f, width, height, 0.1f, 100000.f), frameCounter(0) {
  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    exit(-1);
  }
  // load the shaders
  shader.loadShaders();

  // CONFIG
  glViewport(0, 0, window.getWidth(), window.getHeight());
  glClearColor(0.2f, 0.2f, 0.5f, 1.f);
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

  // Get the verts
  if (!cfg.mesh.path.empty()) {
    registry.getMesh(obj).mesh =
        resourceManager.loadMesh(cfg.mesh.path, cfg.mesh.name);
  } else if (!cfg.sweep.points.empty()) {
    registry.getMesh(obj).mesh = resourceManager.loadMesh(
        cfg.sweep.points, cfg.sweep.resolution, cfg.sweep.radius);
  } else {
    std::cerr << "Error loading object" << std::endl;
  }

  registry.getTransform(obj) = cfg.transform;

  if (cfg.light.intensity != 0.f) {
    registry.getLight(obj) = cfg.light;
  }
  if (cfg.sineAnim.amplitude != 0.f) {
    registry.getSineAnimator(obj) = cfg.sineAnim;
  }
  if (cfg.rotationAnim.rpm != 0.f) {
    registry.getRotationAnimator(obj) = cfg.rotationAnim;
  }
  if (!cfg.parAnim.points.empty()) {
    registry.getParametricAnimator(obj) = cfg.parAnim;
  }
}

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  shader.addUniform("model");
  shader.addUniform("ourTexture");

  // bind uniforms to shader
  shader.bindUniformBlock("LightBlock", 0);
  shader.bindUniformBlock("CameraBlock", 1);

  lightUniformBuffer.bindToPoint(0);
  cameraUniformBuffer.bindToPoint(1);

  std::vector<ObjectConfig> objectConfigs = {

      {.sweep = {{{0, 5, 0},     // start
                  {20, 5, 20},   // curve out
                  {40, 25, 0},   // climb up
                  {20, 40, -20}, // peak & drop
                  {0, 20, -40},  // swoop down
                  {-20, 5, -20}, // valley
                  {-40, 15, 0},  // rise again
                  {-20, 30, 20}, // crest
                  {0, 5, 0}},    // rejoin start (cyclic)
                 1,
                 160}},
      {
          .mesh = {"../../Sync/3dEngine-assets/3d-cubes/", "cube-tex.obj"},
          .transform = {{0, 0, 0}, {0, 0, 0}, {2, 2, 2}},
          .light = {{1, 0, 0}, 500.0f},
          .parAnim = {{{0, 5, 0},     // start
                       {20, 5, 20},   // curve out
                       {40, 25, 0},   // climb up
                       {20, 40, -20}, // peak & drop
                       {0, 20, -40},  // swoop down
                       {-20, 5, -20}, // valley
                       {-40, 15, 0},  // rise again
                       {-20, 30, 20}, // crest
                       {0, 5, 0}},    // rejoin start (cyclic)
                      1},
      },
      {
          .mesh = {"../../Sync/3dEngine-assets/3d-cubes/", "cube-tex.obj"},
          .transform = {{-40, 10, 0}, {0, 0, 0}, {1, 1, 1}},
          .light = {{0, 1, 0}, 500.0f},
          .rotationAnim = {{0, 1, 0}, 30.0f}, // orbit around Y axis
      },
      {
          .mesh = {"../../Sync/3dEngine-assets/3d-cubes/", "cube-tex.obj"},
          .transform = {{0, 10, 40}, {0, 0, 0}, {1, 1, 1}},
          .light = {{0, 0, 1}, 500.0f},
          .rotationAnim = {{0, 1, 0}, 30.0f}, // orbit around Y axis
      },
      {
          .mesh = {"../../Sync/3dEngine-assets/3d-cubes/", "cube-tex.obj"},
          .transform = {{0, 10, -40}, {0, 0, 0}, {1, 1, 1}},
          .light = {{1, 1, 0}, 500.0f},
          .rotationAnim = {{0, 1, 0}, 30.0f}, // orbit around Y axis
      },
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
    // fps(deltaTime);

    // clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // update camera
    moveCamera(deltaTime);

    // Update and render via ECS systems
    updateTransforms(registry);
    updateAnimations(registry, deltaTime);

    // Collect all lights into uniform buffer
    LightBlock lightBlock{};
    lightBlock.count = 0;
    for (size_t i = 0; i < registry.entityCount(); i++) {
      auto &lightOpt = registry.getLight(i);
      if (lightOpt.has_value() && lightBlock.count < MAX_LIGHTS) {
        auto &light = lightOpt.value();
        auto &pos = registry.getTransform(i).position;
        lightBlock.lights[lightBlock.count].position = pos;
        lightBlock.lights[lightBlock.count].color = light.color;
        lightBlock.lights[lightBlock.count].intensity = light.intensity;
        lightBlock.count++;
      }
    }

    // ensure its bounded
    lightUniformBuffer.bindToPoint(0);
    // Upload light data to GPU
    lightUniformBuffer.uploadData(&lightBlock, sizeof(LightBlock));

    CameraBlock cameraBlock{
        camera.getViewMatrix(),
        camera.getProjectionMatrix(),
    };

    // ensure its bounded
    cameraUniformBuffer.bindToPoint(1);
    // Upload light data to GPU
    cameraUniformBuffer.uploadData(&cameraBlock, sizeof(CameraBlock));

    renderAll(registry, shader.getUniformLocation("model"));
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
