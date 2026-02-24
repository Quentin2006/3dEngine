#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <optional>
#include <string>
#include <vector>

class Mesh;

struct MeshComp {
  std::shared_ptr<Mesh> mesh;
};

struct Transform {
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;
  glm::mat4 matrix{1.f};
};

struct Light {
  glm::vec3 color;
  float intensity;
};

struct SineAnimator {
  glm::vec3 axis;
  float amplitude;
  float frequency;
  float phase;
};

struct RotationAnimator {
  glm::vec3 axis;
  float rpm;
};

struct ParametricAnimator {
  std::vector<glm::vec3> points;
  float speed;
  float phase;
};

// NOTE: These are used to generate the above
struct Sweep {
  std::vector<glm::vec3> points;
  float radius;
  int pathSegments;   // # of subdivisions along the spline curve
  int circleSegments; // # of vertices per cross-section circle
};

struct MeshPath {
  std::string path;
  std::string name;
};
