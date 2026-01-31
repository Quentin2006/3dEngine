#include "object.h"
#include "../include/glad/glad.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>

Object::Object()
    : buffer(), position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1),
      modelMatrix(1), vertexCount(0) {}

void Object::setPosition(const glm::vec3 &pos) { position = pos; }

void Object::setRotation(const glm::vec3 &rot) { rotation = rot; }

void Object::setScale(const glm::vec3 &s) { scale = s; }

void Object::loadVertices(const std::vector<glm::vec3> &vertices) {
  buffer.uploadVertices(vertices);
  vertexCount = vertices.size();
}

void Object::updateModelMatrix() {
  modelMatrix = glm::mat4(1.0f);

  modelMatrix = glm::translate(modelMatrix, position);

  // Apply rotation using Euler angles (converted to radians)
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));

  // Apply scale
  modelMatrix = glm::scale(modelMatrix, scale);
}

void Object::draw() {
  // Bind VAO and draw the object
  glBindVertexArray(buffer.getVAO());

  // Draw the geometry
  glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}
