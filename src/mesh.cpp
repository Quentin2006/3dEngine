#include "mesh.h"
#include "../include/glad/glad.h"
#include "vertexBuffer.h"

#include <glm/glm.hpp>

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
