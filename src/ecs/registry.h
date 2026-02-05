#pragma once
#include "components.h"
#include <vector>

class Registry {
public:
  int createEntity() {
    int id = transforms.size();
    transforms.push_back({});
    meshes.push_back({});
    lights.push_back({});
    return id;
  }

  Transform &getTransform(int entity) { return transforms[entity]; }
  MeshComp &getMesh(int entity) { return meshes[entity]; }
  std::optional<Light> &getLight(int entity) { return lights[entity]; }

  size_t entityCount() const { return transforms.size(); }

  std::vector<Transform> &allTransforms() { return transforms; }
  std::vector<MeshComp> &allMeshes() { return meshes; }
  std::vector<std::optional<Light>> &allLights() { return lights; }

private:
  std::vector<Transform> transforms;
  std::vector<MeshComp> meshes;
  std::vector<std::optional<Light>> lights;
};
