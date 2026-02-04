#pragma once

#include "mesh.h"
#include <glm/glm.hpp>
#include <memory>

class Object {
public:
  Object();

  // Prevent copying
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

  // Set transform - same API as before
  void setPosition(const glm::vec3 &pos);
  void setRotation(const glm::vec3 &rot);
  void setScale(const glm::vec3 &scale);

  // Add to transform - same API as before
  void addPosition(const glm::vec3 &pos);
  void addRotation(const glm::vec3 &rot);
  void addScale(const glm::vec3 &scale);

  // Update the model matrix from current transform
  void updateModelMatrix();

  // Bind VAO and draw this object
  void draw();

  // Convenience - loads mesh and assigns it (creates new mesh)
  int loadMesh(const std::string &assetsPath, const std::string &objName);

  // Component management
  void setMesh(std::shared_ptr<Mesh> mesh);
  std::shared_ptr<Mesh> getMesh() const;

  // Getters - same API as before
  const glm::mat4 &getModelMatrix() const { return modelMatrix; }
  unsigned int getVAO();
  size_t getVertexCount() const;

private:
  // Transform data
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::mat4 modelMatrix;

  // Components
  std::shared_ptr<Mesh> mesh;
};
