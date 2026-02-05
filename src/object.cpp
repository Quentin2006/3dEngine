#include "object.h"
#include "../include/glad/glad.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

Registry g_registry;

Object::Object() : entityId(g_registry.createEntity()) {}

void Object::setPosition(const glm::vec3 &pos) {
  g_registry.getTransform(entityId).position = pos;
}

void Object::setRotation(const glm::vec3 &rot) {
  g_registry.getTransform(entityId).rotation = rot;
}

void Object::setScale(const glm::vec3 &s) {
  g_registry.getTransform(entityId).scale = s;
}

void Object::addPosition(const glm::vec3 &pos) {
  g_registry.getTransform(entityId).position += pos;
}

void Object::addRotation(const glm::vec3 &rot) {
  g_registry.getTransform(entityId).rotation += rot;
}

void Object::addScale(const glm::vec3 &s) {
  g_registry.getTransform(entityId).scale += s;
}

void Object::updateModelMatrix() {
  auto &t = g_registry.getTransform(entityId);
  t.matrix = glm::mat4(1.0f);
  t.matrix = glm::translate(t.matrix, t.position);
  t.matrix =
      glm::rotate(t.matrix, glm::radians(t.rotation.x), glm::vec3(1, 0, 0));
  t.matrix =
      glm::rotate(t.matrix, glm::radians(t.rotation.y), glm::vec3(0, 1, 0));
  t.matrix =
      glm::rotate(t.matrix, glm::radians(t.rotation.z), glm::vec3(0, 0, 1));
  t.matrix = glm::scale(t.matrix, t.scale);
}

void Object::draw() {
  auto &meshComp = g_registry.getMesh(entityId);
  if (meshComp.mesh) {
    meshComp.mesh->draw();
  }
}

void Object::update(unsigned int uniformLocation) {
  updateModelMatrix();
  glUniformMatrix4fv(uniformLocation, 1, GL_FALSE,
                     glm::value_ptr(g_registry.getTransform(entityId).matrix));
}

const glm::mat4 &Object::getModelMatrix() const {
  return g_registry.getTransform(entityId).matrix;
}

const glm::vec3 &Object::getPosition() const {
  return g_registry.getTransform(entityId).position;
}

unsigned int Object::getVAO() {
  auto &meshComp = g_registry.getMesh(entityId);
  if (meshComp.mesh) {
    return meshComp.mesh->getVAO();
  }
  return 0;
}

size_t Object::getVertexCount() const {
  auto &meshComp = g_registry.getMesh(entityId);
  if (meshComp.mesh) {
    return meshComp.mesh->getVertexCount();
  }
  return 0;
}

void Object::setMesh(std::shared_ptr<Mesh> m) {
  g_registry.getMesh(entityId).mesh = m;
}

std::shared_ptr<Mesh> Object::getMesh() const {
  return g_registry.getMesh(entityId).mesh;
}

int Object::loadMesh(const std::string &assetsPath,
                     const std::string &objName) {
  auto mesh = std::make_shared<Mesh>();
  int verts = mesh->loadObj(assetsPath, objName);
  setMesh(mesh);
  return verts;
}

void Object::makeLight(const glm::vec3 &color) {
  g_registry.getLight(entityId) = Light{color};
}

bool Object::isLight() const {
  return g_registry.getLight(entityId).has_value();
}

glm::vec3 Object::getLightColor() const {
  auto &lightOpt = g_registry.getLight(entityId);
  if (lightOpt.has_value()) {
    return lightOpt->color;
  }
  return glm::vec3(1, 1, 1);
}
