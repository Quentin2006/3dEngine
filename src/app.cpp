#include "app.h"
#include "callbacks.h"
#include "camera.h"
#include "ecs/components.h"
#include "ecs/registry.h"
#include "ecs/systems.h"
#include "gen.h"
#include "mesh.h"
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
#include <memory>
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

  cameras.push_back(
      std::shared_ptr<Camera>(new Camera(45.f, width, height, 0.1f, 10000.f)));
  cameraIndex = 0;

  glViewport(0, 0, window.getWidth(), window.getHeight());
  glClearColor(0.2f, 0.2f, 0.5f, 1.f);
  glEnable(GL_DEPTH_TEST);

  // glEnable(GL_CULL_FACE);
  // glCullFace(GL_BACK);
  // glFrontFace(GL_CCW);

  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  glfwSetKeyCallback(window.getGLFWwindow(), key_callback);
  glfwSetFramebufferSizeCallback(window.getGLFWwindow(),
                                 framebuffer_size_callback);
}

void App::loadObjectFromConfig(const ObjectConfig &cfg) {
  int obj = registry.createEntity();

  if (!cfg.mesh.path.empty()) {
    auto mesh = resourceManager.loadMesh(cfg.mesh.path, cfg.mesh.name,
                                         cfg.mesh.texturePath);
    registry.setMesh(obj, std::optional<MeshComp>({mesh}));

  } else if (!cfg.sweep.points.empty()) {
    auto mesh = resourceManager.loadMesh(
        cfg.sweep.points, cfg.sweep.pathSegments, cfg.sweep.circleSegments,
        cfg.sweep.radius, cfg.sweep.color);
    registry.setMesh(obj, std::optional<MeshComp>({mesh}));
  }

  registry.setTransform(obj, cfg.transform);

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
    ParametricAnimator pa = cfg.parAnim;
    pa.initialRotation = cfg.transform.rotation;
    registry.setParametricAnimator(obj, std::optional<ParametricAnimator>(pa));
  }
  if (cfg.isCam) {
    ++cameraIndex;
    registry.setCamera(obj, std::shared_ptr<Camera>(cameras[cameraIndex]));
  }
}

void App::run() {
  shader.addUniform("model");
  shader.addUniform("diffuseTexture");
  shader.addUniform("specularTexture");
  shader.addUniform("imageTexture");
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
      // ========= NOTE: Setting up car w/ camera =========
      // coaster cart
      createObject()
          .withMesh("/home/qscheetz/Sync/3dEngine-assets/Amusement "
                    "Park/RollerCoaster/source/",
                    "model (1).obj")
          .withTransform(glm::vec3{0, 0, 0}, glm::vec3{0, 90, 0},
                         glm::vec3{3, 3, 3})
          .withParametricAnimator(coasterPoints, 1, 1.f)
          .build(),

      // Cube orbiting the coaster cart
      createObject()
          .withMesh("/home/qscheetz/Sync/3dEngine-assets/3d-cubes/", "cube.obj")
          .withTransform({2, 0, 0}, {0, 0, 0}, {0.5, 0.5, 0.5}, 0)
          .withRotationAnimator({0, 1, 0}, 20)
          .withSineAnimator({0, 1, 0}, 0.5, 3, 0)
          .build(),

      // cam
      createObject()
          .withTransform({0, 1, 0}, {0, 0, 0}, {1, 1, 1}, 0)
          .withCamera(cameras, 45.f, window.getWidth(), window.getHeight())
          .build(),

      // roller coaster
      createObject()
          .withSweep({coasterPoints, COASTER_RADIUS, COASTER_PATH_SEGMENTS,
                      COASTER_CIRCLE_SEGMENTS, COASTER_COLOR})
          .build(),

      // sun
      createObject()
          .withMesh(
              "/home/qscheetz/Sync/3dEngine-assets/Amusement Park/Sky/sol/",
              "sol.obj",
              "/home/qscheetz/Sync/3dEngine-assets/Amusement "
              "Park/Sky/sol/2k_sun.jpg")
          .withTransform({0, 900, 0}, {0, 0, 0}, {.001, .001, .001})
          .withRotationAnimator({0, 1, 0}, 10)
          .build(),

      // suns light
      createObject()
          .withLight({1, 1, 1}, 100.f)
          .withTransform({0, -50, 0}, {0, 0, 0}, {1, 1, 1}, 3)
          .build(),

      // floor
      createObject()
          .withMesh("/home/qscheetz/Sync/3dEngine-assets/Amusement "
                    "Park/Floor/",
                    "plane.obj",
                    "/home/qscheetz/Sync/3dEngine-assets/Amusement "
                    "Park/Floor/grass.jpg")
          .withTransform(glm::vec3{0, 0, 0}, glm::vec3{0, 0, 0},
                         glm::vec3{WORLD_WIDTH + 5, 1, WORLD_WIDTH + 5})
          .build(),

      // "skybox like"
      createObject()
          .withMesh(
              "/home/qscheetz/Sync/3dEngine-assets/3d-cubes/", "cube-tex.obj",
              "/home/qscheetz/Sync/3dEngine-assets/Amusement Park/Sky/sky.jpg")
          .withTransform({-1000, -1000, -1000}, {0, 0, 0}, {2000, 2000, 2000})
          .build(),

  };
  // ADD COASTER SUPPORTS
  for (const auto &cfg : genRailsForCoaster(coasterPoints, RAIL_COUNT)) {
    objectConfigs.push_back(cfg);
  }

  // ADD COASTER LIGHTS
  for (const auto &cfg : genLightsForCoaster(coasterPoints, 5, {1, 1, 1})) {
    objectConfigs.push_back(cfg);
  }
  for (const auto &cfg :
       genLightsForCoaster(coasterPoints, 5, {0, 1, 1}, .5f)) {
    objectConfigs.push_back(cfg);
  }

  // MAKE TREES
  for (int i = -WORLD_WIDTH / 2; i < WORLD_WIDTH / 2; i += 2) {
    for (int j = -WORLD_WIDTH / 2; j < WORLD_WIDTH / 2; j += 2) {
      glm::vec3 pos = glm::vec3{i * 10, 0, j * 10};
      glm::vec3 randOffset = glm::vec3{rand() % 10 - 5, 0, rand() % 10 - 5};
      glm::vec3 offset = pos + randOffset;

      float randomHeight = TREE_HEIGHT_SCALE + rand() % 3;
      float randomWidth = TREE_BASE_WIDTH + ((rand() * 10) % 3) / 10.f;

      for (const auto &cfg : genTree(offset, randomHeight, randomWidth, 4, 2)) {
        objectConfigs.push_back(cfg);
      }
    }
  }
  std::cerr << "Loading meshes: " << objectConfigs.size() << std::endl;
  for (const auto &cfg : objectConfigs) {
    loadObjectFromConfig(cfg);
  }

  loadObjectFromConfig(createObject()
                           .withLight({1, 0, 0}, .5f)
                           .withTransform({0, 50, 0}, {0, 0, 0}, {1, 1, 1})
                           .build());

  terrainV1 = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
  terrainV2 = {{0.0f, -1.f, 3.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}};
  terrainV3 = {{5.0f, 0.3f, 0.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}};

  subdivLevel = 0;
  terrainMesh = std::make_shared<Mesh>();
  FractalTerrain fractalTerrain;
  terrainMesh->loadVertices(fractalTerrain.generateTerrain(
      subdivLevel, terrainV1, terrainV2, terrainV3));

  terrainEntityId = registry.createEntity();
  registry.setMesh(terrainEntityId, std::optional<MeshComp>({terrainMesh}));
  registry.getMesh(terrainEntityId)
      .value()
      .mesh->setTexture(
          "/home/qscheetz/Sync/3dEngine-assets/Amusement Park/Floor/grass.jpg",
          TextureType::Image);
  registry.setTransform(terrainEntityId,
                        {{0, 20, 30}, {0, 0, 0}, {10, 10, 10}, -1});

  // Light above terrain piece
  loadObjectFromConfig(
      createObject().withTransform({0, 35, 30}, {0, 0, 0}, {1, 1, 1}).build());

  std::cerr << "Subdivision level: " << subdivLevel << " (press +/- to change)"
            << std::endl;

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

    updateAnimations(registry, deltaTime);
    updateTransforms(registry);
    updateCamera(registry);

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
        cameras[cameraIndex]->getViewMatrix(),
        cameras[cameraIndex]->getProjectionMatrix(),
    };

    cameraUniformBuffer.bindToPoint(1);
    cameraUniformBuffer.uploadData(&cameraBlock, sizeof(CameraBlock));

    glUniform3fv(shader.getUniformLocation("cameraPos"), 1,
                 glm::value_ptr(cameras[cameraIndex]->getPosition()));

    shader.use();
    renderAll(registry, shader.getUniformLocation("model"),
              shader.getUniformLocation("imageTexture"),
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
    cameras[cameraIndex]->moveForward(moveAmount);
  if (input.s)
    cameras[cameraIndex]->moveForward(-moveAmount);
  if (input.a)
    cameras[cameraIndex]->moveRight(-moveAmount);
  if (input.d)
    cameras[cameraIndex]->moveRight(moveAmount);
  if (input.q)
    cameras[cameraIndex]->moveUp(moveAmount);
  if (input.e)
    cameras[cameraIndex]->moveUp(-moveAmount);
  if (input.up)
    cameras[cameraIndex]->rotatePitch(rotAmount);
  if (input.down)
    cameras[cameraIndex]->rotatePitch(-rotAmount);
  if (input.left)
    cameras[cameraIndex]->rotateYaw(-rotAmount);
  if (input.right)
    cameras[cameraIndex]->rotateYaw(rotAmount);
  if (input.c && !input.c_pressed) {
    cameraIndex = (cameraIndex + 1) % cameras.size();
    input.c_pressed = true;
  } else if (!input.c) {
    input.c_pressed = false;
  }

  //
  if (input.subdivUp && !input.subdivUp_pressed) {
    if (subdivLevel < 11) {
      subdivLevel++;
      regenerateTerrain();
    }
    input.subdivUp_pressed = true;
  } else if (!input.subdivUp) {
    input.subdivUp_pressed = false;
  }

  if (input.subdivDown && !input.subdivDown_pressed) {
    if (subdivLevel > 0) {
      subdivLevel--;
      regenerateTerrain();
    }
    input.subdivDown_pressed = true;
  } else if (!input.subdivDown) {
    input.subdivDown_pressed = false;
  }
}

void App::regenerateTerrain() {
  FractalTerrain fractalTerrain;
  auto verts = fractalTerrain.generateTerrain(subdivLevel, terrainV1, terrainV2,
                                              terrainV3);
  int count = terrainMesh->loadVertices(verts);
  std::cerr << "Subdivision level: " << subdivLevel << " (" << count
            << " vertices)" << std::endl;
}
