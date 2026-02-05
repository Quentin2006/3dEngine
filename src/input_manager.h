#pragma once

#include <GLFW/glfw3.h>
#include <functional>
#include <optional>
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

  // Process a key event - called by App's key callback
  void processKeyEvent(int key, int action);

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
