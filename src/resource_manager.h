#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Mesh;
class Shader;

class ResourceManager {
public:
  ResourceManager() = default;

  // Load mesh from file, returns cached copy if already loaded
  std::shared_ptr<Mesh> loadMesh(const std::string &path,
                                 const std::string &filename,
                                 unsigned int textureUniform = 0);

private:
  std::unordered_map<std::string, std::weak_ptr<Mesh>> meshCache;
};
