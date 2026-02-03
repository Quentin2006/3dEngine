#ifndef OBJECT_H
#define OBJECT_H

#include "buffer.h"
#include <glm/glm.hpp>
#include <string>

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

  // Update the model matrix from current transform
  void updateModelMatrix();

  // Bind VAO and draw this object
  void draw();

  // We expect the assetsPath the be the dir where all the assets are,
  //  and the objName is the name of the obj file
  int loadObj(const std::string &assetsPath, const std::string &objName);

  // Getters
  const glm::mat4 &getModelMatrix() const { return modelMatrix; }
  unsigned int getVAO() { return buffer.getVAO(); }
  size_t getVertexCount() const { return vertexCount; }

private:
  void setTexture(const std::string &path);

  Buffer buffer;         // Holds VAO/VBO with geometry
  glm::vec3 position;    // World position
  glm::vec3 rotation;    // Rotation in degrees (Euler angles)
  glm::vec3 scale;       // Scale factors
  glm::mat4 modelMatrix; // Combined transform matrix
  size_t vertexCount;    // Number of vertices to draw
  unsigned int texture;

  std::vector<Vertex> vertices;
};

#endif // OBJECT_H
