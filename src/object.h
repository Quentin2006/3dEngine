#ifndef OBJECT_H
#define OBJECT_H

#include "buffer.h"
#include <glm/glm.hpp>

class Object {
public:
  Object();

  // Prevent copying (Buffer is immobile)
  Object(const Object &) = delete;
  Object &operator=(const Object &) = delete;

  // Set transform
  void setPosition(const glm::vec3 &pos);
  void setRotation(const glm::vec3 &rot);
  void setScale(const glm::vec3 &scale);

  // Add to transform
  void addPosition(const glm::vec3 &pos);
  void addRotation(const glm::vec3 &rot);
  void addScale(const glm::vec3 &scale);

  // Load vertex data into the object's buffer
  void loadVertices(const std::vector<glm::vec3> &vertices);

  // Update the model matrix from current transform
  void updateModelMatrix();

  // Bind VAO and draw this object
  void draw();

  // Getters
  const glm::mat4 &getModelMatrix() const { return modelMatrix; }
  unsigned int getVAO() { return buffer.getVAO(); }
  size_t getVertexCount() const { return vertexCount; }

private:
  Buffer buffer;         // Holds VAO/VBO with geometry
  glm::vec3 position;    // World position
  glm::vec3 rotation;    // Rotation in degrees (Euler angles)
  glm::vec3 scale;       // Scale factors
  glm::mat4 modelMatrix; // Combined transform matrix
  size_t vertexCount;    // Number of vertices to draw
};

#endif // OBJECT_H
