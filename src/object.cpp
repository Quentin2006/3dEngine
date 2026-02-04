#include "object.h"
#include "../include/glad/glad.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

Object::Object()
    : position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1), modelMatrix(1) {}

void Object::setPosition(const glm::vec3 &pos) { position = pos; }

void Object::setRotation(const glm::vec3 &rot) { rotation = rot; }

void Object::setScale(const glm::vec3 &s) { scale = s; }

void Object::addPosition(const glm::vec3 &pos) { position += pos; }

void Object::addRotation(const glm::vec3 &rot) { rotation += rot; }

void Object::addScale(const glm::vec3 &s) { scale += s; }

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
  if (mesh) {
    mesh->draw();
  }
}

void Object::setMesh(std::shared_ptr<Mesh> m) { mesh = m; }

std::shared_ptr<Mesh> Object::getMesh() const { return mesh; }

int Object::loadMesh(const std::string &assetsPath,
                     const std::string &objName) {
  mesh = std::make_shared<Mesh>();
  return mesh->loadObj(assetsPath, objName);
}
