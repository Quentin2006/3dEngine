#pragma once
#include "../../include/glad/glad.h"
#include "../mesh.h"
#include "registry.h"
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

inline void updateTransforms(Registry &reg) {
  for (auto &t : reg.allTransforms()) {
    t.matrix = glm::mat4(1.0f);
    t.matrix = glm::translate(t.matrix, t.position);
    t.matrix =
        glm::rotate(t.matrix, glm::radians(t.rotation.x), glm::vec3(1, 0, 0));
    t.matrix =
        glm::rotate(t.matrix, glm::radians(t.rotation.y), glm::vec3(0, 1, 0));
    t.matrix =
        glm::rotate(t.matrix, glm::radians(t.rotation.z), glm::vec3(0, 0, 1));
    t.matrix = glm::scale(t.matrix, t.scale);
  }
}

inline void updateAnimations(Registry &reg, float deltaTime) {
  static float totalTime = 0;
  totalTime += deltaTime;
  for (size_t i = 0; i < reg.entityCount(); i++) {
    // NOTE: CHECK SINE ANIMATOR
    auto &sineOpt = reg.getSineAnimator(i);
    if (sineOpt.has_value()) {
      auto &anim = sineOpt.value();
      auto &t = reg.getTransform(i);
      float offset =
          sin(totalTime * anim.frequency + anim.phase) * anim.amplitude;
      t.position += anim.axis * offset;
    }
    // NOTE: CHECK ROTATION ANIMATOR
    auto &rotOpt = reg.getRotationAnimator(i);
    if (rotOpt.has_value()) {
      auto &anim = rotOpt.value();
      auto &t = reg.getTransform(i);
      t.rotation += anim.axis * anim.rpm * 6.0f * deltaTime;
    }
    // NOTE: CHECK PARAMETRIC ANIMATOR
    auto &parametricOpt = reg.getParametricAnimator(i);
    if (parametricOpt.has_value()) {
      // will generate a point given the time
      auto &anim = parametricOpt.value();

      auto cur_point =
          anim.points[int(totalTime * anim.speed) % anim.points.size()];

      auto &t = reg.getTransform(i);
      t.position = cur_point;
    }
  }
}

inline void renderAll(Registry &reg, GLuint modelUniform) {

  for (size_t i = 0; i < reg.entityCount(); i++) {
    auto &meshComp = reg.getMesh(i);
    if (meshComp.mesh) {
      glUniformMatrix4fv(modelUniform, 1, GL_FALSE,
                         glm::value_ptr(reg.getTransform(i).matrix));
      meshComp.mesh->draw();
    }
  }
}
