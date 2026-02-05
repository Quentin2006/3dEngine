#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <optional>

class Mesh;

struct Transform {
    glm::vec3 position{0, 0, 0};
    glm::vec3 rotation{0, 0, 0};
    glm::vec3 scale{1, 1, 1};
    glm::mat4 matrix{1.0f};
};

struct MeshComp {
    std::shared_ptr<Mesh> mesh;
};

struct Light {
    glm::vec3 color{1, 1, 1};
    float intensity = 1.0f;
};