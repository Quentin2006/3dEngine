#include "objectBuilder.h"

ObjectBuilder &ObjectBuilder::withMesh(const std::string &path,
                                       const std::string &name,
                                       const std::string &texturePath) {
  config.mesh = {
      path,
      name,
      texturePath,
  };
  return *this;
}

ObjectBuilder &ObjectBuilder::withTransform(const glm::vec3 &offset,
                                            const glm::vec3 &rot,
                                            const glm::vec3 &scale,
                                            int parentId) {
  config.transform = {offset, rot, scale, parentId};
  return *this;
}

ObjectBuilder &ObjectBuilder::withSineAnimator(const glm::vec3 &axis, float amp,
                                               float freq, float phase) {
  config.sineAnim = {axis, amp, freq, phase};
  return *this;
}

ObjectBuilder &ObjectBuilder::withRotationAnimator(const glm::vec3 &axis,
                                                   float rpm) {
  config.rotationAnim = {axis, rpm};
  return *this;
}

ObjectBuilder &
ObjectBuilder::withParametricAnimator(const std::vector<glm::vec3> &points,
                                      float speed, float phase) {
  config.parAnim = {points, speed, phase};
  return *this;
}

ObjectBuilder &ObjectBuilder::withLight(const glm::vec3 &color,
                                        float intensity) {
  config.light = {color, intensity};
  return *this;
}

ObjectBuilder &ObjectBuilder::withSweep(const Sweep &sweep) {
  config.sweep = sweep;
  return *this;
}

ObjectConfig ObjectBuilder::build() { return config; }

ObjectBuilder createObject() { return ObjectBuilder{}; }
