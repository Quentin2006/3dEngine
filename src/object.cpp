#include "object.h"
#include "../include/glad/glad.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "../include/tol/tiny_obj_loader.h"

Object::Object()
    : buffer(), position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1),
      modelMatrix(1), vertexCount(0) {}

void Object::setPosition(const glm::vec3 &pos) { position = pos; }

void Object::setRotation(const glm::vec3 &rot) { rotation = rot; }

void Object::setScale(const glm::vec3 &s) { scale = s; }

void Object::addPosition(const glm::vec3 &pos) { position += pos; }

void Object::addRotation(const glm::vec3 &rot) { rotation += rot; }

void Object::addScale(const glm::vec3 &s) { scale += s; }

void Object::updateModelMatrix() {
  modelMatrix = glm::mat4(1.0f);

  modelMatrix = glm::translate(modelMatrix, position);

  // Apply rotation using Euler angles (converted to radians)
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(rotation.x), glm::vec3(1, 0, 0));
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(rotation.y), glm::vec3(0, 1, 0));
  modelMatrix =
      glm::rotate(modelMatrix, glm::radians(rotation.z), glm::vec3(0, 0, 1));

  // std::cerr << glm::to_string(scale) << std::endl;
  // Apply scale
  modelMatrix = glm::scale(modelMatrix, scale);
}

void Object::draw() {
  // Bind VAO and draw the object
  glBindVertexArray(buffer.getVAO());

  glBindTexture(GL_TEXTURE_2D, texture);

  // Draw the geometry
  glDrawArrays(GL_TRIANGLES, 0, vertexCount);
}

void Object::setTexture(const std::string &path) {
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  // set the texture wrapping/filtering options (on the currently bound texture
  // object)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // load and generate the texture
  int width, height, nrChannels;
  unsigned char *data =
      stbi_load(path.c_str(), &width, &height, &nrChannels, 0);

  if (data) {
    GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << "Failed to load texture: " << path << std::endl;
  }
  stbi_image_free(data);
}

int Object::loadObj(const std::string &objFilePath) {
  tinyobj::ObjReader reader;
  tinyobj::ObjReaderConfig readerConfig;
  // readerConfig.mtl_search_path = mtlSearchPath;

  if (!reader.ParseFromFile(objFilePath, readerConfig)) {
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
  // auto &materials = reader.GetMaterials();

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces(polygon)
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        Vertex vertex;
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
        tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
        vertex.position = glm::vec3(vx, vy, vz);

        // Check if `normal_index` is zero or positive. negative = no normal
        // data
        if (idx.normal_index >= 0) {
          tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
          tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
          tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
          vertex.normal = glm::vec3(nx, ny, nz);
        }

        // Check if `texcoord_index` is zero or positive. negative = no texcoord
        // data
        if (idx.texcoord_index >= 0) {
          tinyobj::real_t tx =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
          tinyobj::real_t ty =
              attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
          vertex.texCoord = glm::vec2(tx, ty);
        }

        // Optional: vertex colors
        tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
        tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
        tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
        vertex.color = glm::vec3(red, green, blue);

        vertices.push_back(vertex);
      }
      index_offset += fv;
    }
  }
  buffer.uploadVertices(vertices);
  vertexCount = vertices.size();
  return vertexCount;
}
