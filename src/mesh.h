#pragma once

#include "vertexBuffer.h"
#include <string>
#include <vector>

struct subMesh {
  std::vector<Vertex> vertices;
  unsigned int texture;
};

class Mesh {
public:
  Mesh();

  // Prevent copying (vertexBuffer is immobile)
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  // Draw the mesh
  void draw();

  // Load mesh from OBJ file
  int loadObj(const std::string &assetsPath, const std::string &objFileName);

  // Getters
  size_t getVertexCount() const { return vertexCount; }
  unsigned int getVAO() { return buffer.getVAO(); }

private:
  void setTexture(const std::string &path);

  vertexBuffer buffer;
  size_t vertexCount;
  unsigned int texture;

  std::vector<Vertex> vertices;
};
