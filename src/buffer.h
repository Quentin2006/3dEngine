#include <cstddef>
#include <vector>

class Buffer {
public:
  Buffer();
  ~Buffer();

  void loadVertices(const std::vector<float> &vertices);

  unsigned int getVAO() { return VAO; }

private:
  unsigned int VAO, VBO;
};
