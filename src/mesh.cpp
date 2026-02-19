#include "mesh.h"
#include "../include/glad/glad.h"
#include "vertexBuffer.h"

#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/vector_float2.hpp>
#include <glm/ext/vector_float3.hpp>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/trigonometric.hpp>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../include/tol/tiny_obj_loader.h"

#include <iostream>

Mesh::Mesh(unsigned int textureUniform)
    : buffer(), vertexCount(0), texture(textureUniform) {
  // Create default white texture (1x1 pixel)
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  unsigned char whitePixel[] = {255, 255, 255, 255}; // RGBA white
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
               whitePixel);
}

void Mesh::draw() {
  glBindVertexArray(buffer.getVAO());
  glBindTexture(GL_TEXTURE_2D, texture);
  glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Mesh::setTexture(const std::string &path) {
  // Delete old texture if exists
  if (texture != 0) {
    glDeleteTextures(1, &texture);
  }

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  int width, height, nrChannels;
  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    std::cerr << "Loading texture: " << path << std::endl;
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;

    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);

    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << "Failed to load texture: " << path << std::endl;
    // Fallback to white texture
    unsigned char whitePixel[] = {255, 255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                 whitePixel);
  }
  stbi_image_free(data);
}

int Mesh::loadObj(const std::string &filePath, const std::string &objFileName) {
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig readerConfig;

  if (!reader.ParseFromFile(filePath + objFileName, readerConfig)) {
    if (!reader.Error().empty()) {
      std::cerr << "TinyObjReader: " << reader.Error() << std::endl;
    }
    return 0;
  }

  if (!reader.Warning().empty()) {
    std::cout << "TinyObjReader: " << reader.Warning();
  }

  auto &attrib = reader.GetAttrib();
  auto &shapes = reader.GetShapes();
  auto &materials = reader.GetMaterials();

  // Track if we loaded a texture for this mesh
  bool textureLoaded = false;

  for (size_t s = 0; s < shapes.size(); s++) {
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {

      int material_id = shapes[s].mesh.material_ids[f];

      // Only load texture if material has diffuse texture and we haven't loaded
      // it yet
      if (material_id >= 0 && !textureLoaded) {
        const std::string &texName = materials[material_id].diffuse_texname;
        if (!texName.empty()) {
          std::string texturePath = filePath + texName;
          std::cerr << "Loading texture: " << texturePath << std::endl;
          setTexture(texturePath);
          textureLoaded = true;
        }
      }

      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
      for (size_t v = 0; v < fv; v++) {
        Vertex vertex;
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        vertex.position = glm::vec3(vx, vy, vz);

        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
          vertex.normal = glm::vec3(nx, ny, nz);
        } else {
          if (fv >= 3) {
            tinyobj::index_t idx0 = shapes[s].mesh.indices[index_offset + 0];
            tinyobj::index_t idx1 = shapes[s].mesh.indices[index_offset + 1];
            tinyobj::index_t idx2 = shapes[s].mesh.indices[index_offset + 2];

            glm::vec3 v0(attrib.vertices[3 * idx0.vertex_index + 0],
                         attrib.vertices[3 * idx0.vertex_index + 1],
                         attrib.vertices[3 * idx0.vertex_index + 2]);
            glm::vec3 v1(attrib.vertices[3 * idx1.vertex_index + 0],
                         attrib.vertices[3 * idx1.vertex_index + 1],
                         attrib.vertices[3 * idx1.vertex_index + 2]);
            glm::vec3 v2(attrib.vertices[3 * idx2.vertex_index + 0],
                         attrib.vertices[3 * idx2.vertex_index + 1],
                         attrib.vertices[3 * idx2.vertex_index + 2]);

            glm::vec3 edge1 = v1 - v0;
            glm::vec3 edge2 = v2 - v0;
            vertex.normal = glm::normalize(glm::cross(edge1, edge2));
          } else {
            vertex.normal = glm::vec3(0, 0, 1);
          }
        }

        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty =
              1 - attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
          vertex.texCoord = glm::vec2(tx, ty);
        }

        vertices.push_back(vertex);
      }
      index_offset += fv;
    }
  }

  buffer.uploadVertices(vertices);
  vertexCount = vertices.size();
  return vertexCount;
}

int Mesh::loadSweep(const std::vector<glm::vec3> &points, const size_t res,
                    const float radius) {
  std::vector<glm::vec3> circle_points = generateCircle(res, radius);

  // circles[i] corresponds to the circle centered at points[i]
  std::vector<std::vector<glm::vec3>> circles(points.size());

  // want to translate the vec3's to be centered around the points

  // go thoguh each point
  for (int i = 0; i < (int)points.size(); ++i) {
    std::vector<glm::vec3> translated_circle(res);
    // go though each point in cricles_point
    for (int j = 0; j < (int)circle_points.size(); ++j) {
      translated_circle[j] = circle_points[j] + points[i];
    }
    circles[i] = translated_circle;
  }

  // connect the circles and create the tris
  for (int i = 1; i < (int)circles.size(); ++i) {
    for (int j = 0; j < (int)res; ++j) {
      int next_j = (j + 1) % res;

      glm::vec3 p0 = circles[i - 1][j];
      glm::vec3 p1 = circles[i][j];
      glm::vec3 p2 = circles[i][next_j];
      glm::vec3 p3 = circles[i - 1][next_j];

      // Triangle 1: p0, p1, p2
      glm::vec3 edge1 = p1 - p0;
      glm::vec3 edge2 = p2 - p0;
      glm::vec3 normal1 = glm::normalize(glm::cross(edge1, edge2));

      vertices.push_back({p0, glm::vec2(0.0f), normal1});
      vertices.push_back({p1, glm::vec2(0.0f), normal1});
      vertices.push_back({p2, glm::vec2(0.0f), normal1});

      // Triangle 2: p0, p2, p3
      glm::vec3 edge3 = p2 - p0;
      glm::vec3 edge4 = p3 - p0;
      glm::vec3 normal2 = glm::normalize(glm::cross(edge3, edge4));

      vertices.push_back({p0, glm::vec2(0.0f), normal2});
      vertices.push_back({p2, glm::vec2(0.0f), normal2});
      vertices.push_back({p3, glm::vec2(0.0f), normal2});
    }
  }

  buffer.uploadVertices(vertices);
  vertexCount = vertices.size();
  return vertexCount;
}

const std::vector<glm::vec3> Mesh::generateCircle(int res, float radius) {
  std::vector<glm::vec3> circle_points(res);

  // We want to go 360/res points from the circle untill we hit 360
  float cur_degree = 0;
  for (auto &point : circle_points) {
    point = glm::vec3(cos(glm::radians(cur_degree)) * radius, 0,
                      sin(glm::radians(cur_degree)) * radius);

    cur_degree += 360.f / res;
  }

  return circle_points;
}
