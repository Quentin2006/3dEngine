#include "app.h"
#include "callbacks.h"
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

void key_callback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
  App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
  if (!app) return;

  bool pressed = (action == GLFW_PRESS);
  bool released = (action == GLFW_RELEASE);

  if (!pressed && !released) return;

  InputState* input = app->getInputState();
  switch (key) {
    case Controls::MOVE_FORWARD: input->w = pressed; return;
    case Controls::MOVE_BACKWARD: input->s = pressed; return;
    case Controls::MOVE_LEFT: input->a = pressed; return;
    case Controls::MOVE_RIGHT: input->d = pressed; return;
    case Controls::MOVE_DOWN: input->q = pressed; return;
    case Controls::MOVE_UP: input->e = pressed; return;
    case Controls::ROTATE_PITCH_UP: input->up = pressed; return;
    case Controls::ROTATE_PITCH_DOWN: input->down = pressed; return;
    case Controls::ROTATE_YAW_LEFT: input->left = pressed; return;
    case Controls::ROTATE_YAW_RIGHT: input->right = pressed; return;
    case Controls::NEXT_CAMERA: input->c = pressed; return;
    default: return;
  }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
  if (!app) return;

  app->getWindow()->setWidth(width);
  app->getWindow()->setHeight(height);

  for (auto& camera : *app->getCameras()) {
    camera.updateAspect(width, height);
  }

  auto& cams = *app->getCameras();
  auto* shader = app->getShader();
  glUniformMatrix4fv(
      shader->getUniformLocation("projection"), 1, GL_FALSE,
      glm::value_ptr(cams[app->getCameraIndex()].getProjectionMatrix()));

  glViewport(0, 0, width, height);
}

ObjectBuilder& ObjectBuilder::withMesh(const std::string& path, const std::string& name) {
  config.mesh = {path, name};
  return *this;
}

ObjectBuilder& ObjectBuilder::withTransform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, int parentId) {
  config.transform = {pos, rot, scale, parentId};
  return *this;
}

ObjectBuilder& ObjectBuilder::withSineAnimator(const glm::vec3& axis, float amp, float freq, float phase) {
  config.sineAnim = {axis, amp, freq, phase};
  return *this;
}

ObjectBuilder& ObjectBuilder::withRotationAnimator(const glm::vec3& axis, float rpm) {
  config.rotationAnim = {axis, rpm};
  return *this;
}

ObjectBuilder& ObjectBuilder::withParametricAnimator(const std::vector<glm::vec3>& points, float speed, float phase) {
  config.parAnim = {points, speed, phase};
  return *this;
}

ObjectBuilder& ObjectBuilder::withCamera(float fov) {
  config.camera = {fov};
  return *this;
}

ObjectBuilder& ObjectBuilder::withLight(const glm::vec3& color, float intensity) {
  config.light = {color, intensity};
  return *this;
}

ObjectBuilder& ObjectBuilder::withSweep(const Sweep& sweep) {
  config.sweep = sweep;
  return *this;
}

ObjectConfig ObjectBuilder::build() {
  return config;
}

ObjectBuilder createObject() {
  return ObjectBuilder{};
}

void fps(float deltaTime);

App::App(int width, int height, const std::string &title)
    : window(width, height, title), shader(), frameCounter(0) {

  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    exit(-1);
  }
  // load the shaders
  ShaderResult shaderLoadResult = shader.loadShaders();
  if (shaderLoadResult != ShaderResult::Success) {
    std::cerr << "Failed to load shaders: " << static_cast<int>(shaderLoadResult) << std::endl;
    exit(1);
  }

  // init camera
  cameras.push_back(Camera(45.f, width, height));
  cameraIndex = 0;

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

  if (cfg.camera.FOV > 0.f) {
    registry.getCamera(obj) =
        Camera(cfg.camera.FOV, window.getWidth(), window.getWidth());
  }

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
      createObject()
          .withSweep({coasterPoints, COASTER_PATH_SEGMENTS, 3000, COASTER_CIRCLE_SEGMENTS, {1, 0, 1}})
          .build(),

      createObject()
          .withMesh("../../Sync/3dEngine-assets/Car/", "Car.obj")
          .withTransform({0, 20, 0}, {0, 0, 0}, {COASTER_CAR_SCALE, COASTER_CAR_SCALE, COASTER_CAR_SCALE}, -1)
          .withParametricAnimator(coasterPoints, COASTER_CAR_SPEED, 0.f)
          .withCamera(45.f)
          .build(),
  };

  // add lights to objectConfigs
  for (const auto &cfg : genLightsForCoaster(coasterPoints, LIGHT_COUNT)) {
    objectConfigs.push_back(cfg);
  }

  for (const auto &cfg : genTree(glm::vec3{0, 0, 0}, TREE_HEIGHT_SCALE, TREE_BASE_WIDTH, TREE_NUM_LEVELS, TREE_NUM_PER_LEVEL)) {
    objectConfigs.push_back(cfg);
  }

  for (const auto &cfg : genRailsForCoaster(coasterPoints, RAIL_COUNT)) {
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
        cameras[cameraIndex].getViewMatrix(),
        cameras[cameraIndex].getProjectionMatrix(),
    };

    // ensure its bounded
    cameraUniformBuffer.bindToPoint(1);
    // Upload light data to GPU
    cameraUniformBuffer.uploadData(&cameraBlock, sizeof(CameraBlock));

    // Set camera position for specular lighting
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
  if (input.c) {
    cameraIndex = (cameraIndex + 1) % cameras.size();
    std::cerr << cameraIndex << std::endl;
  }
}
