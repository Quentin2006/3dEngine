#include "buffer.h"
#include "../include/glad/glad.h"

Buffer::Buffer() {
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

Buffer::~Buffer() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
}

void Buffer::loadVertices(const std::vector<glm::vec3> &vertices) {
  // 1. Bind the specific buffer we created in the constructor
  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // 2. Upload the data
  // CORRECT SIZE CALCULATION: number of elements * size of one element
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3),
               vertices.data(), GL_STATIC_DRAW);

  // 3. Unbind the buffer (good practice)
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}
