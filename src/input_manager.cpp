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

void InputManager::processKeyEvent(int key, int action) {
  bool pressed = (action == GLFW_PRESS);
  bool released = (action == GLFW_RELEASE);

  if (!pressed && !released) {
    return;
  }

  auto actionOpt = getActionForKey(key);
  if (!actionOpt.has_value()) {
    return;
  }

  InputAction inputAction = actionOpt.value();
  
  if (pressed) {
    actionStates[inputAction] = true;
    actionPressedThisFrame[inputAction] = true;
  } else if (released) {
    actionStates[inputAction] = false;
  }
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
