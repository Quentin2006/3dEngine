#include "app.h"
#include "callbacks.h"
#include "ecs/components.h"
#include "ecs/registry.h"
#include "ecs/systems.h"
#include "gen.h"
#include "objectBuilder.h"
#include "resource_manager.h"
#include "uniformBuffer.h"
#include <GLFW/glfw3.h>
#include <chrono>
#include <cstdlib>
#include <glm/common.hpp>
#include <glm/ext/scalar_constants.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/ext/vector_float4.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <ostream>
#include <vector>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

void fps(float deltaTime);

App::App(int width, int height, const std::string &title)
    : window(width, height, title), shader(), frameCounter(0) {

  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    exit(-1);
  }
  ShaderResult shaderLoadResult = shader.loadShaders();
  if (shaderLoadResult != ShaderResult::Success) {
    std::cerr << "Failed to load shaders: "
              << static_cast<int>(shaderLoadResult) << std::endl;
    exit(1);
  }

  cameras.push_back(Camera(45.f, width, height));
  cameraIndex = 0;

  glViewport(0, 0, window.getWidth(), window.getHeight());
  glClearColor(0.2f, 0.2f, 0.5f, 1.f);
  glEnable(GL_DEPTH_TEST);

  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  glfwSetKeyCallback(window.getGLFWwindow(), key_callback);
  glfwSetFramebufferSizeCallback(window.getGLFWwindow(),
                                 framebuffer_size_callback);
}

void App::loadObjectFromConfig(const ObjectConfig &cfg) {
  int obj = registry.createEntity();

  if (!cfg.mesh.path.empty()) {
    auto mesh = resourceManager.loadMesh(cfg.mesh.path, cfg.mesh.name);
    registry.setMesh(obj, std::optional<MeshComp>({mesh}));

  } else if (!cfg.sweep.points.empty()) {
    auto mesh = resourceManager.loadMesh(
        cfg.sweep.points, cfg.sweep.pathSegments, cfg.sweep.circleSegments,
        cfg.sweep.radius, cfg.sweep.color);
    registry.setMesh(obj, std::optional<MeshComp>({mesh}));
  }

  registry.setTransform(obj, cfg.transform);

  if (cfg.camera.FOV > 0.f) {
    registry.setCamera(
        obj, Camera(cfg.camera.FOV, window.getWidth(), cfg.camera.FOV));
    cameras.push_back(registry.getCamera(obj).value());
  }

  if (cfg.light.intensity != 0.f) {
    registry.setLight(obj, std::optional<Light>(cfg.light));
  }
  if (cfg.sineAnim.amplitude != 0.f) {
    registry.setSineAnimator(obj, std::optional<SineAnimator>(cfg.sineAnim));
  }
  if (cfg.rotationAnim.rpm != 0.f) {
    registry.setRotationAnimator(
        obj, std::optional<RotationAnimator>(cfg.rotationAnim));
  }
  if (!cfg.parAnim.points.empty()) {
    registry.setParametricAnimator(
        obj, std::optional<ParametricAnimator>(cfg.parAnim));
  }
}

void App::run() {
  shader.addUniform("model");
  shader.addUniform("diffuseTexture");
  shader.addUniform("specularTexture");
  shader.addUniform("shininess");
  shader.addUniform("cameraPos");

  shader.bindUniformBlock("LightBlock", 0);
  shader.bindUniformBlock("CameraBlock", 1);

  lightUniformBuffer.bindToPoint(0);
  cameraUniformBuffer.bindToPoint(1);

  std::vector<glm::vec3> coasterPoints = {
      {30, 5, 0},    {20, 8, 15},  {0, 50, 20},  {-20, 8, 15}, {-30, 5, 0},
      {-20, 8, -15}, {0, 12, -20}, {20, 8, -15}, {30, 5, 0}};

  std::vector<ObjectConfig> objectConfigs = {
      createObject()
          .withSweep({coasterPoints,
                      COASTER_PATH_SEGMENTS,
                      3000,
                      COASTER_CIRCLE_SEGMENTS,
                      {1, 0, 1}})
          .build(),

      createObject()
          .withMesh("../../Sync/3dEngine-assets/Car/", "Car.obj")
          .withTransform(
              {0, 20, 0}, {0, 0, 0},
              {COASTER_CAR_SCALE, COASTER_CAR_SCALE, COASTER_CAR_SCALE}, -1)
          .withParametricAnimator(coasterPoints, COASTER_CAR_SPEED, 0.f)
          .withCamera(45.f)
          .build(),

      createObject()
          .withMesh("../../Sync/3dEngine-assets/test/", "Untitled.obj")
          .withTransform({20, 0, 20}, {0, 0, 0}, {10, 10, 10}, -1)
          .build(),
  };

  for (const auto &cfg : genLightsForCoaster(coasterPoints, LIGHT_COUNT)) {
    objectConfigs.push_back(cfg);
  }

  for (const auto &cfg :
       genTree(glm::vec3{0, 0, 0}, TREE_HEIGHT_SCALE, TREE_BASE_WIDTH,
               TREE_NUM_LEVELS, TREE_NUM_PER_LEVEL)) {
    objectConfigs.push_back(cfg);
  }

  for (const auto &cfg : genRailsForCoaster(coasterPoints, RAIL_COUNT)) {
    objectConfigs.push_back(cfg);
  }

  std::cerr << "Loading meshes: " << objectConfigs.size() << std::endl;
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

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    moveCamera(deltaTime);

    updateTransforms(registry);
    updateAnimations(registry, deltaTime);

    LightBlock lightBlock{};
    lightBlock.count = 0;
    for (auto &id : registry.getLightEntityIds()) {
      auto &lightOpt = registry.getLight(id);
      auto &light = lightOpt.value();
      auto &pos = registry.getTransform(id).position;

      lightBlock.lights[lightBlock.count].position = pos;
      lightBlock.lights[lightBlock.count].color = light.color;
      lightBlock.lights[lightBlock.count].intensity = light.intensity;
      lightBlock.count++;
    }

    lightUniformBuffer.bindToPoint(0);
    lightUniformBuffer.uploadData(&lightBlock, sizeof(LightBlock));

    CameraBlock cameraBlock{
        cameras[cameraIndex].getViewMatrix(),
        cameras[cameraIndex].getProjectionMatrix(),
    };

    cameraUniformBuffer.bindToPoint(1);
    cameraUniformBuffer.uploadData(&cameraBlock, sizeof(CameraBlock));

    glUniform3fv(shader.getUniformLocation("cameraPos"), 1,
                 glm::value_ptr(cameras[cameraIndex].getPosition()));

    shader.use();
    renderAll(registry, shader.getUniformLocation("model"),
              shader.getUniformLocation("diffuseTexture"),
              shader.getUniformLocation("specularTexture"),
              shader.getUniformLocation("shininess"));
    window.swapBuffers();
  }

  glfwTerminate();
}

void App::moveCamera(float deltaTime) {
  float moveAmount = MOVEMENT_SPEED * deltaTime;
  float rotAmount = ROTATION_SPEED * deltaTime;

  if (input.w)
    cameras[cameraIndex].moveForward(moveAmount);
  if (input.s)
    cameras[cameraIndex].moveForward(-moveAmount);
  if (input.a)
    cameras[cameraIndex].moveRight(-moveAmount);
  if (input.d)
    cameras[cameraIndex].moveRight(moveAmount);
  if (input.q)
    cameras[cameraIndex].moveUp(moveAmount);
  if (input.e)
    cameras[cameraIndex].moveUp(-moveAmount);
  if (input.up)
    cameras[cameraIndex].rotatePitch(rotAmount);
  if (input.down)
    cameras[cameraIndex].rotatePitch(-rotAmount);
  if (input.left)
    cameras[cameraIndex].rotateYaw(-rotAmount);
  if (input.right)
    cameras[cameraIndex].rotateYaw(rotAmount);
  if (input.c && !input.c_pressed) {
    cameraIndex = (cameraIndex + 1) % cameras.size();
    input.c_pressed = true;
  } else if (!input.c) {
    input.c_pressed = false;
  }
}
