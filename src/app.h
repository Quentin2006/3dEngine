#pragma once

#include "../include/glad/glad.h"
#include "camera.h"
#include "ecs/components.h"
#include "ecs/registry.h"
#include "resource_manager.h"
#include "shader.h"
#include "uniformBuffer.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
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
};

struct ObjectConfig {
  MeshComp meshObj{nullptr};
  MeshPath mesh{"", ""};
  Transform transform{
      {0, 0, 0},
      {0, 0, 0},
      {1, 1, 1},
      -1,
  };
  Light light{{0, 0, 0}, 0.f};
  SineAnimator sineAnim{{0, 0, 0}, 0.f, 0.f, 0.f};
  RotationAnimator rotationAnim{{0, 0, 0}, 0.f};
  Sweep sweep{{}, 0, 0, 0, {1, 1, 1}};
  ParametricAnimator parAnim{{}, 0.f, 0.f};
  CameraConf camera{-1.f};
};

struct Controls {
  // Camera movement
  static const int MOVE_FORWARD = GLFW_KEY_W;  // Move camera forward
  static const int MOVE_BACKWARD = GLFW_KEY_S; // Move camera backward
  static const int MOVE_LEFT = GLFW_KEY_A;     // Move camera left (strafe)
  static const int MOVE_RIGHT = GLFW_KEY_D;    // Move camera right (strafe)
  static const int MOVE_UP = GLFW_KEY_E;       // Move camera up
  static const int MOVE_DOWN = GLFW_KEY_Q;     // Move camera down

  // Camera rotation
  static const int ROTATE_PITCH_UP = GLFW_KEY_UP; // Look up (rotate around X)
  static const int ROTATE_PITCH_DOWN =
      GLFW_KEY_DOWN; // Look down (rotate around X)
  static const int ROTATE_YAW_LEFT =
      GLFW_KEY_LEFT; // Look left (rotate around Y)
  static const int ROTATE_YAW_RIGHT =
      GLFW_KEY_RIGHT; // Look right (rotate around Y)

  // Misc binds
  static const int NEXT_CAMERA = GLFW_KEY_C;
};

class ObjectBuilder {
public:
  ObjectBuilder& withMesh(const std::string& path, const std::string& name);
  ObjectBuilder& withTransform(const glm::vec3& pos, const glm::vec3& rot, const glm::vec3& scale, int parentId = -1);
  ObjectBuilder& withSineAnimator(const glm::vec3& axis, float amp, float freq, float phase);
  ObjectBuilder& withRotationAnimator(const glm::vec3& axis, float rpm);
  ObjectBuilder& withParametricAnimator(const std::vector<glm::vec3>& points, float speed, float phase);
  ObjectBuilder& withCamera(float fov);
  ObjectBuilder& withLight(const glm::vec3& color, float intensity);
  ObjectBuilder& withSweep(const Sweep& sweep);

  ObjectConfig build();

private:
  ObjectConfig config;
};

ObjectBuilder createObject();

class App {
public:
  App(int width, int height, const std::string &title);

  void run();
  void moveCamera(float deltaTime);

  Window *getWindow() { return &window; };
  std::vector<Camera> *getCameras() { return &cameras; };
  InputState *getInputState() { return &input; };
  Shader *getShader() { return &shader; };

  int getCameraIndex() { return cameraIndex; }

private:
  bool loadShaders();
  void loadObjectFromConfig(const ObjectConfig &cfg);
  void loadObjectsFromConfig(const std::vector<ObjectConfig> &configs);

  Window window;
  Shader shader;
  std::vector<Camera> cameras;
  uint32_t cameraIndex;
  InputState input;
  unsigned int frameCounter;
  ResourceManager resourceManager;
  Registry registry;
  UniformBuffer lightUniformBuffer;
  UniformBuffer cameraUniformBuffer;
};
