#pragma once

#include "ecs/components.h"
#include <glm/vec3.hpp>
#include <optional>
#include <string>
#include <vector>

struct ObjectConfig {
  MeshComp meshObj{nullptr};
  MeshPath mesh{"", ""};
  Transform transform{
      {0, 0, 0},
      {0, 0, 0},
      {1, 1, 1},
      -1,
  };
  Light light{{0, 0, 0}, 0.f};
  SineAnimator sineAnim{{0, 0, 0}, 0.f, 0.f, 0.f};
  RotationAnimator rotationAnim{{0, 0, 0}, 0.f};
  Sweep sweep{{}, 0, 0, 0, {1, 1, 1}};
  ParametricAnimator parAnim{{}, 0.f, 0.f};
};

class ObjectBuilder {
public:
  ObjectBuilder &withMesh(const std::string &path, const std::string &name);
  ObjectBuilder &withTransform(const glm::vec3 &pos, const glm::vec3 &rot,
                               const glm::vec3 &scale, int parentId = -1);
  ObjectBuilder &withSineAnimator(const glm::vec3 &axis, float amp, float freq,
                                  float phase);
  ObjectBuilder &withRotationAnimator(const glm::vec3 &axis, float rpm);
  ObjectBuilder &withParametricAnimator(const std::vector<glm::vec3> &points,
                                        float speed, float phase);
  ObjectBuilder &withLight(const glm::vec3 &color, float intensity);
  ObjectBuilder &withSweep(const Sweep &sweep);

  ObjectConfig build();

private:
  ObjectConfig config;
};

ObjectBuilder createObject();
