#include "resource_manager.h"
#include "mesh.h"
#include "shader.h"
#include <iostream>

std::shared_ptr<Mesh> ResourceManager::loadMesh(const std::string &path,
                                                const std::string &filename) {
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
  auto mesh = std::make_shared<Mesh>();
  int verts = mesh->loadObj(path, filename);
  if (verts == 0) {
    std::cerr << "Failed to load mesh: " << key << std::endl;
    return nullptr;
  }

  meshCache[key] = mesh;
  return mesh;
}

std::shared_ptr<Shader> ResourceManager::loadShader(const std::string &name) {
  // Check if already cached
  auto it = shaderCache.find(name);
  if (it != shaderCache.end()) {
    if (auto shared = it->second.lock()) {
      return shared;
    }
    // Expired, remove from cache
    shaderCache.erase(it);
  }

  // Load new shader
  // Note: Currently Shader::loadShaders() uses hardcoded paths
  // This would need to be modified to support named shaders
  auto shader = std::make_shared<Shader>();
  shader->loadShaders();

  shaderCache[name] = shader;
  return shader;
}
