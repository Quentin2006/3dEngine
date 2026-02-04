#pragma once

#include "buffer.h"
#include <string>
#include <vector>

class Mesh {
public:
  Mesh();

  // Prevent copying (Buffer is immobile)
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  // Draw the mesh
  void draw();

  // Load mesh from OBJ file
  int loadObj(const std::string &assetsPath, const std::string &objName);

  // Getters
  size_t getVertexCount() const { return vertexCount; }
  unsigned int getVAO() { return buffer.getVAO(); }

private:
  void setTexture(const std::string &path);

  Buffer buffer;
  size_t vertexCount;
  unsigned int texture;

  std::vector<Vertex> vertices;
};
