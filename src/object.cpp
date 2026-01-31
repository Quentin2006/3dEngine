#include "object.h"
#include "../include/glad/glad.h"

#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <iostream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "../include/stb/stb_image.h"

Object::Object()
    : buffer(), position(0, 0, 0), rotation(0, 0, 0), scale(1, 1, 1),
      modelMatrix(1), vertexCount(0) {}

void Object::setPosition(const glm::vec3 &pos) { position = pos; }

void Object::setRotation(const glm::vec3 &rot) { rotation = rot; }

void Object::setScale(const glm::vec3 &s) { scale = s; }

void Object::addPosition(const glm::vec3 &pos) { position += pos; }

void Object::addRotation(const glm::vec3 &rot) { rotation += rot; }

void Object::addScale(const glm::vec3 &s) { scale += s; }

void Object::loadVertices(const std::vector<Vertex> &vertices) {
  buffer.uploadVertices(vertices);
  vertexCount = vertices.size();
}

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

void Object::loadTexture(const std::string &path) {
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
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  } else {
    std::cerr << "Failed to load texture: " << path << std::endl;
  }
  stbi_image_free(data);
}
