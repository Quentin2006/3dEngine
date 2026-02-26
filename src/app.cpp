#include "app.h"
#include "ecs/components.h"
#include "ecs/registry.h"
#include "ecs/systems.h"
#include "gen.h"
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
        cfg.sweep.points, cfg.sweep.pathSegments, cfg.sweep.circleSegments,
        cfg.sweep.radius, cfg.sweep.color);
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
  shader.addUniform("diffuseTexture");
  shader.addUniform("specularTexture");
  shader.addUniform("shininess");
  shader.addUniform("cameraPos");

  // bind uniforms to shader
  shader.bindUniformBlock("LightBlock", 0);
  shader.bindUniformBlock("CameraBlock", 1);

  lightUniformBuffer.bindToPoint(0);
  cameraUniformBuffer.bindToPoint(1);

  std::vector<glm::vec3> coasterPoints = {
      {30, 5, 0},    {20, 8, 15},  {0, 50, 20},  {-20, 8, 15}, {-30, 5, 0},
      {-20, 8, -15}, {0, 12, -20}, {20, 8, -15}, {30, 5, 0}};

  // NOTE: entities will get id's starting at 0
  std::vector<ObjectConfig> objectConfigs = {
      // === ROLLER COASTER TRACK ===
      {.sweep = {coasterPoints, 0.4f, 3000, 24, {1, 0, 1}}},

      {.mesh = {"../../Sync/3dEngine-assets/Car/", "Car.obj"},
       .transform = {{0, 20, 0}, {0, 0, 0}, {.2, .2, .2}},
       .parAnim = {coasterPoints, .5f, 0.f}},

  };

  // add lgihts to objectConfigs
  for (const auto &cfg : genLightsForCoaster(coasterPoints, 10)) {
    objectConfigs.push_back(cfg);
  }

  // for (int i = 0; i < 1; i++) {
  //
  //   auto tree = genTree(glm::vec3{i * 5, i, 0}, 1.0f + i, 0.25f, 4, 7);
  //
  //   for (const auto &cfg : tree) {
  //     objectConfigs.push_back(cfg);
  //   }
  // }

  for (const auto &cfg : genRailsForCoaster(coasterPoints, 10)) {
    objectConfigs.push_back(cfg);
  }

  // LOAD MESHES
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

    // Set camera position for specular lighting
    glUniform3fv(shader.getUniformLocation("cameraPos"), 1,
                 glm::value_ptr(camera.getPosition()));

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
    camera.moveForward(moveAmount);
  if (input.s)
    camera.moveForward(-moveAmount);
  if (input.a)
    camera.moveRight(-moveAmount);
  if (input.d)
    camera.moveRight(moveAmount);
  if (input.q)
    camera.moveUp(moveAmount);
  if (input.e)
    camera.moveUp(-moveAmount);
  if (input.up)
    camera.rotatePitch(rotAmount);
  if (input.down)
    camera.rotatePitch(-rotAmount);
  if (input.left)
    camera.rotateYaw(-rotAmount);
  if (input.right)
    camera.rotateYaw(rotAmount);
}
