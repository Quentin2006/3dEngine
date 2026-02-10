#pragma once

#include <cstddef>
#include <glm/ext/vector_float3.hpp>

// Maximum number of lights supported
constexpr int MAX_LIGHTS = 32;

// Light data structure aligned for std140 layout
// std140 requires vec3 to be aligned to 16 bytes (4 floats)
struct LightData {
  alignas(16) glm::vec3 position;
  alignas(16) glm::vec3 color;
};

// Uniform block structure matching shader layout
// std140 requires arrays to be aligned to vec4 boundaries
struct LightBlock {
  LightData lights[MAX_LIGHTS];
  int count;
  int _padding[3]; // Pad to vec4 alignment
};

class UniformBuffer {
public:
  UniformBuffer();
  ~UniformBuffer();

  // Prevent copying/moving (OpenGL resources must stay in one place)
  UniformBuffer(const UniformBuffer &) = delete;
  UniformBuffer &operator=(const UniformBuffer &) = delete;
  UniformBuffer(UniformBuffer &&) = delete;
  UniformBuffer &operator=(UniformBuffer &&) = delete;

  // Upload data to GPU buffer. Can be called multiple times to update data.
  void uploadData(const void *data, size_t size);

  // Bind buffer to a specific binding point for shaders to access
  void bindToPoint(unsigned int bindingPoint);

  // Get the OpenGL buffer handle
  unsigned int getBuffer() { return UBO; }

private:
  unsigned int UBO; // Uniform Buffer Object handle
};
