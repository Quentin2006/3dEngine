#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <glm/glm.hpp>
#include <glm/trigonometric.hpp>

class Camera {
public:
  Camera(float FOV, int width, int height) {
    position = glm::vec3(0.0f, 0.0f, 3.0f);
    up = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = -90.0f;
    pitch = 0.0f;
    updateFront();
    projection = glm::perspective(glm::radians(FOV),
                                  (float)width / (float)height, 0.1f, 100.0f);
  }

  void move(const glm::vec3 &deltaPos) { position += deltaPos; }
  void moveForward(float distance);
  void moveRight(float distance);
  void moveUp(float distance);
  void rotateYaw(float delta);
  void rotatePitch(float delta);

  glm::mat4 getViewMatrix() {
    return glm::lookAt(position, position + front, up);
  };

  glm::mat4 getProjectionMatrix() { return projection; }

private:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 up;
  glm::mat4 projection;
  float yaw;
  float pitch;

  void updateFront();
};
