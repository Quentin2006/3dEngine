#pragma once

#include "vertexBuffer.h"
#include <glm/detail/qualifier.hpp>
#include <glm/ext/vector_float3.hpp>
#include <string>
#include <vector>

enum TextureType { Diffuse, Specular, Image };

struct Material {
  unsigned int imageTexture;
  unsigned int specularTexture;
  unsigned int diffuseTexture;
  float shininess;
  unsigned char color[4];
  // NOTE: we can add more
};

// FIXME: we want to hold the materaisl for a vertex inside of a vector
//
// Materials[materialID] -> [(imageTexture, specularTexture, diffuseTexture),
// ...]
//  each veretex will point to one of these mnaterials
//
//  in the vertexBuffer, we can just fgetch the matrial proprties then send it
//  over to the gpu

class Mesh {
public:
  Mesh(unsigned int textureUniform = 0, glm::vec3 color = {1.f, 1.f, 1.f});

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
  // float getShininess() const { return shininess; }

private:
  void setTexture(const std::string &path, TextureType type);

  const std::vector<glm::vec3> generateCircle(int res, float radius);

  vertexBuffer buffer;

  // unsigned int imageTexture;
  // unsigned int specularTexture;
  // unsigned int diffuseTexture;
  // float shininess;
  // glm::vec3 color;
  std::vector<Material> materials;

  std::vector<Vertex> vertices;
  size_t vertexCount;
};
