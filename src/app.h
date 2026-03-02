#pragma once

#include "../include/glad/glad.h"
#include "camera.h"
#include "controls.h"
#include "ecs/registry.h"
#include "objectBuilder.h"
#include "resource_manager.h"
#include "shader.h"
#include "uniformBuffer.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <memory>

constexpr float MOVEMENT_SPEED = 15.f;
constexpr float ROTATION_SPEED = 125.f;

constexpr float COASTER_PATH_SEGMENTS = 0.4f;
constexpr int COASTER_CIRCLE_SEGMENTS = 24;
constexpr float COASTER_CAR_SPEED = 0.5f;
constexpr float COASTER_CAR_SCALE = 0.2f;

constexpr float TREE_BASE_WIDTH = 0.25f;
constexpr int TREE_NUM_LEVELS = 4;
constexpr int TREE_NUM_PER_LEVEL = 7;
constexpr float TREE_HEIGHT_SCALE = 3.0f;

constexpr int LIGHT_COUNT = 10;
constexpr int RAIL_COUNT = 10;

struct InputState {
  bool w = false, a = false, s = false, d = false;
  bool q = false, e = false;
  bool up = false, down = false, left = false, right = false;
  bool c = false;
  bool c_pressed = false;
};

class App {
public:
  App(int width, int height, const std::string &title);

  void run();
  void moveCamera(float deltaTime);

  Window *getWindow() { return &window; };
  std::vector<std::shared_ptr<Camera>> *getCameras() { return &cameras; };
  InputState *getInputState() { return &input; };
  Shader *getShader() { return &shader; };

  int getCameraIndex() { return cameraIndex; }

private:
  bool loadShaders();
  void loadObjectFromConfig(const ObjectConfig &cfg);
  void loadObjectsFromConfig(const std::vector<ObjectConfig> &configs);

  Window window;
  Shader shader;
  std::vector<std::shared_ptr<Camera>> cameras;
  uint32_t cameraIndex;
  InputState input;
  unsigned int frameCounter;
  ResourceManager resourceManager;
  Registry registry;
  UniformBuffer lightUniformBuffer;
  UniformBuffer cameraUniformBuffer;
};
