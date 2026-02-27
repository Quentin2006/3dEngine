#pragma once
#include "../camera.h"
#include "components.h"
#include <vector>

class Registry {
public:
  int createEntity() {
    int id = transforms.size();
    transforms.push_back({});
    meshes.push_back({});
    lights.push_back({});
    sineAnimators.push_back({});
    rotationAnimators.push_back({});
    parametricAnimators.push_back({});
    cameras.push_back({});
    return id;
  }

  void destroyEntity(int entity) {
    transforms[entity] = {};
    meshes[entity].mesh.reset();
    lights[entity].reset();
    sineAnimators[entity].reset();
  }

  Transform &getTransform(int entity) { return transforms[entity]; }
  MeshComp &getMesh(int entity) { return meshes[entity]; }
  std::optional<SineAnimator> &getSineAnimator(int entity) {
    return sineAnimators[entity];
  }
  std::optional<RotationAnimator> &getRotationAnimator(int entity) {
    return rotationAnimators[entity];
  }
  std::optional<ParametricAnimator> &getParametricAnimator(int entity) {
    return parametricAnimators[entity];
  }
  std::optional<Light> &getLight(int entity) { return lights[entity]; }
  std::optional<Camera> &getCamera(int entity) { return cameras[entity]; }

  size_t entityCount() const { return transforms.size(); }

  std::vector<Transform> &allTransforms() { return transforms; }
  std::vector<MeshComp> &allMeshes() { return meshes; }
  std::vector<std::optional<Light>> &allLights() { return lights; }
  std::optional<Camera> &allCamera(int entity) { return cameras[entity]; }

private:
  std::vector<Transform> transforms;
  std::vector<MeshComp> meshes;
  std::vector<std::optional<Light>> lights;
  std::vector<std::optional<SineAnimator>> sineAnimators;
  std::vector<std::optional<RotationAnimator>> rotationAnimators;
  std::vector<std::optional<ParametricAnimator>> parametricAnimators;
  std::vector<std::optional<Camera>> cameras;

  std::vector<int> sparse;
};
