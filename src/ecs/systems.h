#pragma once
#include "registry.h"
#include "../mesh.h"
#include "../../include/glad/glad.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

inline void updateTransforms(Registry& reg) {
    for (auto& t : reg.allTransforms()) {
        t.matrix = glm::mat4(1.0f);
        t.matrix = glm::translate(t.matrix, t.position);
        t.matrix = glm::rotate(t.matrix, glm::radians(t.rotation.x), glm::vec3(1, 0, 0));
        t.matrix = glm::rotate(t.matrix, glm::radians(t.rotation.y), glm::vec3(0, 1, 0));
        t.matrix = glm::rotate(t.matrix, glm::radians(t.rotation.z), glm::vec3(0, 0, 1));
        t.matrix = glm::scale(t.matrix, t.scale);
    }
}

inline void renderAll(Registry& reg, GLuint modelUniform) {
    for (size_t i = 0; i < reg.entityCount(); i++) {
        auto& meshComp = reg.getMesh(i);
        if (meshComp.mesh) {
            glUniformMatrix4fv(modelUniform, 1, GL_FALSE,
                             glm::value_ptr(reg.getTransform(i).matrix));
            meshComp.mesh->draw();
        }
    }
}