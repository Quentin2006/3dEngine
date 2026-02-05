#pragma once

#include "ecs/registry.h"
#include "mesh.h"
#include <glm/glm.hpp>
#include <memory>

extern Registry g_registry;

class Object {
public:
    Object();
    ~Object() = default;

    Object(const Object&) = delete;
    Object& operator=(const Object&) = delete;

    Object(Object&&) = default;
    Object& operator=(Object&&) = default;

    void setPosition(const glm::vec3& pos);
    void setRotation(const glm::vec3& rot);
    void setScale(const glm::vec3& scale);

    void addPosition(const glm::vec3& pos);
    void addRotation(const glm::vec3& rot);
    void addScale(const glm::vec3& scale);

    void updateModelMatrix();

    void draw();
    void update(unsigned int uniformLocation);

    int loadMesh(const std::string& assetsPath, const std::string& objName);

    void setMesh(std::shared_ptr<Mesh> mesh);
    std::shared_ptr<Mesh> getMesh() const;

    const glm::mat4& getModelMatrix() const;
    unsigned int getVAO();
    const glm::vec3& getPosition() const;
    size_t getVertexCount() const;

    void makeLight(const glm::vec3& color = glm::vec3(1, 1, 1));
    bool isLight() const;
    glm::vec3 getLightColor() const;

private:
    int entityId;
};