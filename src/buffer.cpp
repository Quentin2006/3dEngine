#include "buffer.h"
#include "../include/glad/glad.h"

Buffer::Buffer() : VAO(0), VBO(0) {
  // Generate OpenGL handles for VAO and VBO
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
}

Buffer::~Buffer() {
  // Only delete if we still own the resources (check for 0)
  if (VBO)
    glDeleteBuffers(1, &VBO);
  if (VAO)
    glDeleteVertexArrays(1, &VAO);
}

void Buffer::uploadVertices(const std::vector<glm::vec3> &vertices) {
  // Skip if no data provided
  if (vertices.empty())
    return;

  // Bind VAO to store all subsequent configuration
  glBindVertexArray(VAO);
  // Bind VBO for data upload and attribute configuration
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // Upload vertex data to GPU buffer
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_STATIC_DRAW);

  // Configure attribute 0: 3 floats per vertex (position), no normalization
  // Stride = sizeof(vec3) to handle potential padding
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);
  // Enable the vertex attribute
  glEnableVertexAttribArray(0);

  // Note: We don't unbind VBO here because VAO needs to remember
  // which VBO is bound for this attribute configuration
}
