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

extern Registry g_registry;

App::App(int width, int height, const std::string &title)
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

  // Setup window user pointer for callbacks
  glfwSetWindowUserPointer(window.getGLFWwindow(), this);
  
  // Setup InputManager
  inputManager.initializeDefaults();
  
  // Setup callbacks
  glfwSetKeyCallback(window.getGLFWwindow(), App::keyCallback);
  glfwSetFramebufferSizeCallback(window.getGLFWwindow(),
                                 framebuffer_size_callback);
}

void App::loadObjectFromConfig(const ObjectConfig &cfg) {
  int obj = g_registry.createEntity();
  g_registry.getMesh(obj).mesh =
      resourceManager.loadMesh(cfg.mesh.path, cfg.mesh.name);
  g_registry.getTransform(obj).position = cfg.transform.pos;
  g_registry.getTransform(obj).rotation = cfg.transform.rot;
  g_registry.getTransform(obj).scale = cfg.transform.scale;

  if (cfg.light.color != glm::vec3(0, 0, 0)) {
    g_registry.getLight(obj) = Light{cfg.light.color, cfg.light.intensity};
  }

  if (cfg.sineAnim.amplitude != 0.f) {
    g_registry.getSineAnimator(obj) = {
        cfg.sineAnim.axis,
        cfg.sineAnim.amplitude,
        cfg.sineAnim.frequency,
        cfg.sineAnim.phase,
    };
  }
}

void App::loadObjectsFromConfig(const std::vector<ObjectConfig> &configs) {
  for (const auto &cfg : configs) {
    loadObjectFromConfig(cfg);
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

void App::run() {
  // get uniform location, now that the shader exists, we can find the ID
  shader.addUniform("model");
  shader.addUniform("view");
  shader.addUniform("projection");
  shader.addUniform("ourTexture");
  shader.addUniform("lightPos");
  shader.addUniform("lightColor");

  const std::vector<ObjectConfig> objectConfigs = {
      {{"assets/human/", "FinalBaseMesh.obj"}},

      {{"assets/Car/", "Car.obj"},
       {{0, 0, 0}, 1.0f},
       {{10, 0, 10}, {0, 0, 0}, {.1, .1, .1}}},

      {{"assets/3d-cubes/", "cube-tex.obj"},
       {{1, 1, 1}, 1.0f},
       {{0, 0, 0}, {0, 0, 0}, {1, 1, 1}},
       {{1, 0, 0}, .01, 1.5, 1}},

      {{"assets/wolf/", "Wolf_obj.obj"},
       {{0, 0, 0}, 1.0f},
       {{10, 5, 1}, {0, 0, 0}, {2, 2, 2}}},
  };

  loadObjectsFromConfig(objectConfigs);

  auto prevTime = std::chrono::steady_clock::now();
  float totalTime;

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
    updateTransforms(g_registry);
    updateAnimations(g_registry, totalTime);

    renderAll(g_registry, shader.getUniformLocation("model"),
              shader.getUniformLocation("lightPos"),
              shader.getUniformLocation("lightColor"));

    window.swapBuffers();
    
    // Reset input pressed states for next frame
    inputManager.endFrame();
  }

  glfwTerminate();
}

// would like to abstract into camera class
void App::moveCamera(float deltaTime) {
  if (inputManager.isActionActive(InputAction::MOVE_FORWARD))
    camera.moveForward(MOVEMENT_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::MOVE_BACKWARD))
    camera.moveForward(-MOVEMENT_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::MOVE_LEFT))
    camera.moveRight(-MOVEMENT_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::MOVE_RIGHT))
    camera.moveRight(MOVEMENT_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::MOVE_UP))
    camera.moveUp(-MOVEMENT_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::MOVE_DOWN))
    camera.moveUp(MOVEMENT_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::ROTATE_PITCH_UP))
    camera.rotatePitch(ROTATION_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::ROTATE_PITCH_DOWN))
    camera.rotatePitch(-ROTATION_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::ROTATE_YAW_LEFT))
    camera.rotateYaw(-ROTATION_SPEED * deltaTime);
  if (inputManager.isActionActive(InputAction::ROTATE_YAW_RIGHT))
    camera.rotateYaw(ROTATION_SPEED * deltaTime);
  
  // Handle one-shot action for shader reload
  if (inputManager.wasActionPressed(InputAction::RELOAD_SHADERS)) {
    std::cerr << "Reloading shaders" << std::endl;
    shader.loadShaders();
  }
}

void App::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));
  
  if (!app)
    return;
  
  app->inputManager.processKeyEvent(key, action);
}
