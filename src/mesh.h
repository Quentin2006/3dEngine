#pragma once

#include "vertexBuffer.h"
#include <glm/ext/vector_float3.hpp>
#include <string>
#include <vector>

struct subMesh {
  std::vector<Vertex> vertices;
  unsigned int texture;
};

enum TextureType { Diffuse, Specular, Image };

class Mesh {
public:
  Mesh(unsigned int textureUniform = 0);

  // Prevent copying (vertexBuffer is immobile)
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  // Draw the mesh
  void draw();

  // Load mesh from OBJ file
  int loadObj(const std::string &assetsPath, const std::string &objFileName);

  // loads mesh from sweep config
  int loadSweep(const std::vector<glm::vec3> &points, int pathSegments,
                int circleSegments, float radius);
  // Getters
  size_t getVertexCount() const { return vertexCount; }
  unsigned int getVAO() { return buffer.getVAO(); }
  float getShininess() const { return shininess; }

private:
  void setTexture(const std::string &path, TextureType type);

  const std::vector<glm::vec3> generateCircle(int res, float radius);

  vertexBuffer buffer;
  size_t vertexCount;
  unsigned int imageTexture;
  unsigned int specularTexture;
  unsigned int diffuseTexture;
  float shininess;

  std::vector<Vertex> vertices;
};
