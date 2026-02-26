#include "app.h"
#include "ecs/components.h"
#include "ecs/registry.h"
#include "ecs/systems.h"
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

// === NOTE: Helper functions ===

std::vector<ObjectConfig>
genLightsForCoaster(const std::vector<glm::vec3> &coasterPoints, int count) {
  std::vector<ObjectConfig> coasterLights = {};

  for (int i = 0; i < count; i++) {
    float phase = (float)i / count;
    coasterLights.push_back(
        {.light = {{1, 1, 1}, 1}, .parAnim = {coasterPoints, .5f, phase}});
  }

  return coasterLights;
}

std::vector<ObjectConfig> genRailsForCoaster(const std::vector<glm::vec3> &pts,
                                             int count) {
  std::vector<ObjectConfig> coasterRails = {};

  if (pts.size() < 2) {
    return {};
  }

  for (int i = 0; i < count; i++) {
    float phase = (float)i / count;
    int numSegments = (int)pts.size() - 1;
    float t = std::fmod(numSegments * phase, numSegments);

    // Handle negative time by wrapping to positive range
    if (t < 0.0f)
      t += (float)numSegments;

    auto [pos, _] = spline::calculatePosOnSpline(true, numSegments, pts, t);

    // NOTE: not sure why we need to divide by 2
    const glm::vec3 floorPos = {pos.x / 2, 0.f, pos.z / 2};
    const glm::vec3 topPos = {pos.x / 2, pos.y * 1.05, pos.z / 2};

    coasterRails.push_back({
        .transform = {floorPos, {0, 0, 0}, {1, 1, 1}, -1, 1.f},
        .sweep = {{floorPos, topPos}, 0.25, 10, 20, {0.7f, 0.7f, 0.7f}},
    });
  }

  return coasterRails;
}

// will generate a tree with numBranches and numLeaves at pos
// FIXME: this needs to be memozied or tabulated for better preformance
void genTree(const glm::vec3 &startPos, const glm::vec3 &endPos, float width,
             int numLevels, int numPerLevel,
             std::vector<ObjectConfig> &resTree) {

  if (numLevels <= 0) {
    return;
  }

  glm::vec3 lightBrown = {0.76, 0.60, 0.42};
  glm::vec3 darkBrown = {0.36, 0.22, 0.12};
  glm::vec3 color = glm::mix(lightBrown, darkBrown, float(rand() % 100) / 100);

  int circleSegments = std::max(3, 3 + numLevels * 3);

  // add main log
  if (resTree.size() == 0) {

    // set to brown if we're not the last branch
    resTree.push_back({
        .transform = {startPos, {0, 0, 0}, {1, 1, 1}, -1, 1.f},
        .sweep = {{startPos, endPos}, width, 10, circleSegments, color},
    });
    width *= 0.3f;
    genTree(startPos, endPos, width * 0.3, numLevels - 1, numPerLevel, resTree);
    return;
  }

  // for this log, add branches
  for (int i = 0; i < numPerLevel; i++) {

    // override color if we're the last branch
    if (numLevels == 1) {
      glm::vec3 lightGreen = {0.12, 0.42, 0.16};
      glm::vec3 darkGreen = {0.55, 0.80, 0.35};

      // set to brown if we're not the last branch
      color = glm::mix(lightGreen, darkGreen, float(rand() % 100) / 100);
    }

    // get random start x,y,z value on the log by linear interpolating
    // want it to start 30% up the startPos and 95% end pos
    glm ::vec3 branchStart =
        glm::mix(glm::mix(startPos, endPos, 0.3f),
                 glm::mix(startPos, endPos, 0.95f), rand() % 100 / 100.f);

    // get random end x,y,z value extending outward from branch start
    float branchLen = glm::distance(startPos, endPos) * 0.5f;

    glm::vec3 randomUp =
        normalize(glm::vec3{rand() % 100, rand() % 100, rand() % 100});

    // rotate direction to random direction
    glm::vec3 direction = branchStart - startPos;

    direction = glm::normalize(glm::cross(randomUp, direction));

    glm::vec3 axis = glm::normalize(glm::vec3{
        float(rand() % 100), float(rand() % 100), float(rand() % 100)});

    direction = glm::normalize(
        glm::rotate(glm::mat4{1.f}, glm::radians<float>(rand() % 360), axis) *
        glm::vec4{direction, 0.f});

    direction = glm::normalize(axis * direction);

    glm::vec3 branchEnd = branchStart + direction * branchLen;

    // add branch to resTree
    resTree.push_back({
        .transform = {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}, -1, 1.f},
        .sweep = {{branchStart, branchEnd}, width, 10, circleSegments, color},
    });

    // for this branch, add leaves
    genTree(branchStart, branchEnd, width * 0.5f, numLevels - 1, numPerLevel,
            resTree);
  }
}

void fps(float deltaTime) {
  static float fpsTimer = 0;
  fpsTimer += deltaTime;

  if (fpsTimer >= 1) {
    std::cerr << "\rfps: " << 1 / deltaTime << std::flush;
    fpsTimer = 0;
  }
}

// === NOTE: End of helper functions ===

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

  };

  // add lgihts to objectConfigs
  // for (const auto &cfg : genLightsForCoaster(coasterPoints, 10)) {
  //   objectConfigs.push_back(cfg);
  // }

  std::vector<ObjectConfig> tree = {};
  genTree(glm::vec3{0, 0, 0}, glm::vec3{0, 5, 0}, .25, 4, 5, tree);
  for (const auto &cfg : tree) {
    objectConfigs.push_back(cfg);
  }

  for (const auto &cfg : genRailsForCoaster(coasterPoints, 10)) {
    objectConfigs.push_back(cfg);
  }

  // LOAD MESHES
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
