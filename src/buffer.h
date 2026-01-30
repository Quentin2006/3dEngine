#include <cstddef>
#include <glm/ext/vector_float3.hpp>
#include <vector>

class Buffer {
public:
  Buffer();
  ~Buffer();

  void loadVertices(const std::vector<glm::vec3> &vertices);

  unsigned int getVAO() { return VAO; }

private:
  unsigned int VAO, VBO;
};
