#include "resource_manager.h"
#include "mesh.h"
#include <iostream>

std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string &path,
                                                const std::string &filename,
                                                unsigned int textureUniform) {
  std::string key = path + filename;

  // Check if already cached
  auto it = meshCache.find(key);
  if (it != meshCache.end()) {
    if (auto shared = it->second.lock()) {
      return shared;
    }
    // Expired, remove from cache
    meshCache.erase(it);
  }

  // Load new mesh
  auto mesh = std::make_shared<Mesh>(textureUniform);
  int verts = mesh->loadObj(path, filename);
  if (verts == 0) {
    std::cerr << "Failed to load mesh: " << key << std::endl;
    return nullptr;
  }

  meshCache[key] = mesh;
  return mesh;
}
