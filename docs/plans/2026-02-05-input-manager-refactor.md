# Input Manager Refactor Implementation Plan

> **For Claude:** REQUIRED SUB-SKILL: Use superpowers:executing-plans to implement this plan task-by-task.

**Goal:** Extract input handling from App into a dedicated InputManager class to improve separation of concerns and code organization.

**Architecture:** Create an InputManager class that encapsulates all input state, key mappings, and callback handling. App will own an InputManager instance and query it for input state instead of managing InputState directly.

**Tech Stack:** C++17, GLFW

---

## Overview of Changes

Current Problems:
- `InputState` and `Controls` structs are defined in `app.h`, polluting the header
- `key_callback` is an inline function in `app.h` that directly manipulates App's input state
- Tight coupling between GLFW callbacks and App's internal state
- No abstraction - App knows too much about input implementation details

Solution:
- Create `InputManager` class that owns input state and handles callbacks
- App queries InputManager for input state via clean API
- Decouples input handling from App logic
- Makes input system reusable and testable

---

## Task 1: Create InputManager Header

**Files:**
- Create: `src/input_manager.h`

**Step 1: Write the header file**

```cpp
#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <unordered_map>

// Actions that can be mapped to keys
enum class InputAction {
  MOVE_FORWARD,
  MOVE_BACKWARD,
  MOVE_LEFT,
  MOVE_RIGHT,
  MOVE_UP,
  MOVE_DOWN,
  ROTATE_PITCH_UP,
  ROTATE_PITCH_DOWN,
  ROTATE_YAW_LEFT,
  ROTATE_YAW_RIGHT,
  RELOAD_SHADERS,
  COUNT
};

// Manages all input state and key mappings
class InputManager {
public:
  InputManager();

  // Initialize with default key mappings
  void initializeDefaults();

  // Check if an action is currently active (key held down)
  bool isActionActive(InputAction action) const;

  // Check if an action was just pressed this frame (for one-shot actions)
  bool wasActionPressed(InputAction action) const;

  // Update input state at end of frame (resets pressed states)
  void endFrame();

  // GLFW callback - static method that forwards to instance
  static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

  // Set the InputManager instance for GLFW callbacks
  void registerWithWindow(GLFWwindow* window);

  // Key mapping configuration
  void bindKey(int glfwKey, InputAction action);
  void unbindKey(int glfwKey);

private:
  // Current state of each action (true = key held)
  std::unordered_map<InputAction, bool> actionStates;
  
  // Actions that were pressed this frame (for one-shot detection)
  std::unordered_map<InputAction, bool> actionPressedThisFrame;
  
  // Key to action mapping
  std::unordered_map<int, InputAction> keyBindings;
  
  // Get action from key, returns std::nullopt if not bound
  std::optional<InputAction> getActionForKey(int key) const;
};
```

**Step 2: Verify the header compiles**

Run: `make -j4`

Expected: May fail with undefined reference errors for InputManager methods (OK - we'll implement next)

---

## Task 2: Implement InputManager

**Files:**
- Create: `src/input_manager.cpp`

**Step 1: Write the implementation**

```cpp
#include "input_manager.h"
#include <iostream>

InputManager::InputManager() {
  // Initialize all actions to false
  for (int i = 0; i < static_cast<int>(InputAction::COUNT); ++i) {
    auto action = static_cast<InputAction>(i);
    actionStates[action] = false;
    actionPressedThisFrame[action] = false;
  }
}

void InputManager::initializeDefaults() {
  // Camera movement
  bindKey(GLFW_KEY_W, InputAction::MOVE_FORWARD);
  bindKey(GLFW_KEY_S, InputAction::MOVE_BACKWARD);
  bindKey(GLFW_KEY_A, InputAction::MOVE_LEFT);
  bindKey(GLFW_KEY_D, InputAction::MOVE_RIGHT);
  bindKey(GLFW_KEY_E, InputAction::MOVE_UP);
  bindKey(GLFW_KEY_Q, InputAction::MOVE_DOWN);

  // Camera rotation
  bindKey(GLFW_KEY_UP, InputAction::ROTATE_PITCH_UP);
  bindKey(GLFW_KEY_DOWN, InputAction::ROTATE_PITCH_DOWN);
  bindKey(GLFW_KEY_LEFT, InputAction::ROTATE_YAW_LEFT);
  bindKey(GLFW_KEY_RIGHT, InputAction::ROTATE_YAW_RIGHT);

  // Actions
  bindKey(GLFW_KEY_R, InputAction::RELOAD_SHADERS);
}

bool InputManager::isActionActive(InputAction action) const {
  auto it = actionStates.find(action);
  return it != actionStates.end() && it->second;
}

bool InputManager::wasActionPressed(InputAction action) const {
  auto it = actionPressedThisFrame.find(action);
  return it != actionPressedThisFrame.end() && it->second;
}

void InputManager::endFrame() {
  // Reset pressed states for next frame
  for (auto& [action, pressed] : actionPressedThisFrame) {
    pressed = false;
  }
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
  // Get the InputManager instance from window user pointer
  auto* inputManager = static_cast<InputManager*>(glfwGetWindowUserPointer(window));
  
  if (!inputManager) {
    return;
  }

  bool pressed = (action == GLFW_PRESS);
  bool released = (action == GLFW_RELEASE);

  if (!pressed && !released) {
    return;
  }

  auto actionOpt = inputManager->getActionForKey(key);
  if (!actionOpt.has_value()) {
    return;
  }

  InputAction inputAction = actionOpt.value();
  
  if (pressed) {
    inputManager->actionStates[inputAction] = true;
    inputManager->actionPressedThisFrame[inputAction] = true;
  } else if (released) {
    inputManager->actionStates[inputAction] = false;
  }
}

void InputManager::registerWithWindow(GLFWwindow* window) {
  glfwSetWindowUserPointer(window, this);
  glfwSetKeyCallback(window, InputManager::keyCallback);
}

void InputManager::bindKey(int glfwKey, InputAction action) {
  keyBindings[glfwKey] = action;
}

void InputManager::unbindKey(int glfwKey) {
  keyBindings.erase(glfwKey);
}

std::optional<InputAction> InputManager::getActionForKey(int key) const {
  auto it = keyBindings.find(key);
  if (it != keyBindings.end()) {
    return it->second;
  }
  return std::nullopt;
}
```

**Step 2: Verify compilation**

Run: `make -j4`

Expected: Should compile without errors (InputManager not yet used)

---

## Task 3: Update App Header

**Files:**
- Modify: `src/app.h`

**Step 1: Remove InputState and Controls structs**

Remove lines 16-67 (InputState struct and Controls struct).

Replace with:
```cpp
#include "input_manager.h"
```

**Step 2: Update App class members**

Change:
```cpp
// Old:
InputState input;

// New:
InputManager inputManager;
```

**Step 3: Update App constructor and methods**

The App constructor will now initialize InputManager.

Updated app.h should look like:

```cpp
#pragma once

#include "../include/glad/glad.h"
#include "camera.h"
#include "input_manager.h"
#include "resource_manager.h"
#include "shader.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <memory>

constexpr float MOVEMENT_SPEED = 15.f;
constexpr float ROTATION_SPEED = 125.f;

struct ObjectConfig {
  struct Mesh {
    std::string path;
    std::string name;
  } mesh{};

  struct Light {
    glm::vec3 color{0, 0, 0};
    float intensity = 1.0f;
  } light{};

  struct Transform {
    glm::vec3 pos{0, 0, 0};
    glm::vec3 rot{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
  } transform{};

  struct SineAnim {
    glm::vec3 axis{1, 0, 0};
    float amplitude = 0.0f;
    float frequency = 1.0f;
    float phase = 0.0f;
  } sineAnim{};
};

class App {
public:
  App(int width, int height, const std::string &title);

  void run();
  void moveCamera(float deltaTime);

  Window window;
  Shader shader;
  Camera camera;
  InputManager inputManager;

private:
  bool loadShaders();
  void loadObjectFromConfig(const ObjectConfig &cfg);
  void loadObjectsFromConfig(const std::vector<ObjectConfig> &configs);

  unsigned int frameCounter;
  ResourceManager resourceManager;
};

// Remove key_callback and framebuffer_size_callback from here
// They'll be handled differently - framebuffer_size_callback stays as static method

inline void framebuffer_size_callback(GLFWwindow *window, int width,
                                      int height) {
  App *app = static_cast<App *>(glfwGetWindowUserPointer(window));

  if (!app)
    return;

  app->window.setWidth(width);
  app->window.setHeight(height);
  app->camera.updateAspect(width, height);

  glUniformMatrix4fv(app->shader.getUniformLocation("projection"), 1, GL_FALSE,
                     glm::value_ptr(app->camera.getProjectionMatrix()));

  glViewport(0, 0, width, height);
}
```

**Step 3: Verify header compiles**

Run: `make -j4`

Expected: Will fail - need to update app.cpp

---

## Task 4: Update App Implementation

**Files:**
- Modify: `src/app.cpp`

**Step 1: Remove key_callback function**

Delete the entire `key_callback` function (it was inline in app.h, now gone).

**Step 2: Update App constructor**

Change constructor initialization:

```cpp
App::App(int width, int height, const std::string &title)
    : window(width, height, title), shader(),
      camera(45.f, width, height, 0.1f, 1000.f), frameCounter(0) {
  if (!gladLoadGLLoader((void *(*)(const char *))glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    exit(-1);
  }
  
  shader.loadShaders();

  glViewport(0, 0, window.getWidth(), window.getHeight());
  glClearColor(0, 0, .1f, 1.f);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);
  glFrontFace(GL_CCW);

  // Setup InputManager
  inputManager.initializeDefaults();
  inputManager.registerWithWindow(window.getGLFWwindow());
  
  // Setup framebuffer callback
  glfwSetFramebufferSizeCallback(window.getGLFWwindow(),
                                 framebuffer_size_callback);
}
```

**Step 3: Update moveCamera() to use InputManager**

Replace the entire `moveCamera` function:

```cpp
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
```

**Step 4: Update run() loop**

Add `inputManager.endFrame()` at the end of the main loop:

```cpp
while (!window.shouldClose()) {
  // ... existing code ...
  
  renderAll(g_registry, shader.getUniformLocation("model"),
            shader.getUniformLocation("lightPos"),
            shader.getUniformLocation("lightColor"));

  window.swapBuffers();
  
  // Reset input pressed states for next frame
  inputManager.endFrame();
}
```

**Step 5: Remove extern InputState reference**

If there are any references to old InputState in app.cpp, remove them.

**Step 6: Verify compilation**

Run: `make -j4`

Expected: Should compile successfully

---

## Task 5: Test the Refactor

**Files:**
- None (testing)

**Step 1: Build the project**

Run: `make clean && make -j4`

Expected: Clean build with no errors or warnings

**Step 2: Run the application**

Run: `./bin/app` (or appropriate executable name)

Expected: 
- Window opens
- Scene renders correctly
- Camera movement works (WASD, QE, arrow keys)
- Shader reload works (R key)
- No crashes or errors

**Step 3: Test edge cases**

- Hold multiple keys simultaneously
- Press keys before window focus
- Resize window (should still work with framebuffer_size_callback)

---

## Task 6: Add InputManager to Makefile

**Files:**
- Modify: `Makefile`

**Step 1: Ensure InputManager is compiled**

The Makefile uses `find` to discover source files, so it should pick up `src/input_manager.cpp` automatically. Verify:

Run: `make clean && make -j4`

Check that `obj/input_manager.o` is created.

---

## Task 7: Commit the Changes

**Files:**
- None (git operation)

**Step 1: Review changes**

Run: `git status`

Expected: 
- New files: `src/input_manager.h`, `src/input_manager.cpp`
- Modified: `src/app.h`, `src/app.cpp`, `Makefile` (maybe)
- New: `docs/plans/2026-02-05-input-manager-refactor.md`

**Step 2: Stage and commit**

```bash
git add src/input_manager.h src/input_manager.cpp
 git add src/app.h src/app.cpp
git add docs/plans/2026-02-05-input-manager-refactor.md
git commit -m "refactor: extract InputManager from App

- Create InputManager class to encapsulate all input handling
- Move InputState and Controls into InputManager
- Decouple input logic from App class
- Add action-based input mapping system
- Support for key rebinding via bindKey/unbindKey
- Maintain backward compatibility with existing controls"
```

---

## Post-Refactor Benefits

1. **Separation of Concerns**: App no longer manages input state directly
2. **Reusability**: InputManager can be used by other classes
3. **Testability**: InputManager can be unit tested independently
4. **Extensibility**: Easy to add new input actions or key mappings
5. **Maintainability**: Input logic is centralized in one place

## Future Enhancements (Out of Scope)

- Mouse input handling
- Gamepad/controller support  
- Input configuration file loading/saving
- Action groups (e.g., UI vs gameplay input modes)
- Input buffering for fighting game-style combos
