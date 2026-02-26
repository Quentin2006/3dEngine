#pragma once
#include "../../include/glad/glad.h"
#include "../math/spline.h"
#include "../mesh.h"
#include "registry.h"
#include <cmath>
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

    if (t.parrentId >= 0) {
      auto &parrentTranform = reg.getTransform(t.parrentId);
      t.matrix = parrentTranform.matrix * t.matrix;
    }
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
    // Get the parametric animator for this entity
    auto &parametricOpt = reg.getParametricAnimator(i);
    if (parametricOpt.has_value()) {
      auto &anim = parametricOpt.value();
      const auto &pts = anim.points;

      // Need at least 2 points to define a path
      if (pts.size() >= 2) {
        // Check if path is closed (first and last points are nearly identical)
        bool cyclic = glm::length(pts.front() - pts.back()) < 0.001f;

        // Number of segments is points minus one
        int numSegments = (int)pts.size() - 1;

        // Calculate normalized position along the entire path
        // totalTime * speed gives us how far along the path we are
        // phase offsets the starting position on the path
        float globalT = std::fmod(totalTime * anim.speed +
                                      numSegments * parametricOpt->phase,
                                  numSegments);
        // Handle negative time by wrapping to positive range
        if (globalT < 0.0f)
          globalT += (float)numSegments;

        auto [pos, tangent] =
            spline::calculatePosOnSpline(cyclic, numSegments, pts, globalT);

        // Interpolate position along the spline
        auto &t = reg.getTransform(i);

        t.position = pos + t.offset;

        // Convert tangent vector to yaw/pitch rotation angles
        // yaw = horizontal direction, pitch = vertical angle
        float yaw = glm::degrees(std::atan2(-tangent.z, tangent.x));
        float pitch =
            glm::degrees(std::asin(glm::clamp(tangent.y, -1.0f, 1.0f)));

        // Apply rotation so entity faces along the path
        t.rotation = {pitch, yaw, 0.0f};
      }
    }
  }
}

inline void renderAll(Registry &reg, GLuint modelUniform, GLint diffuseTexUnit,
                      GLint specularTexUnit, GLuint shininessLoc) {

  for (size_t i = 0; i < reg.entityCount(); i++) {
    auto &meshComp = reg.getMesh(i);
    if (meshComp.mesh) {
      glUniformMatrix4fv(modelUniform, 1, GL_FALSE,
                         glm::value_ptr(reg.getTransform(i).matrix));

      // Set texture units for specular lighting
      glUniform1i(diffuseTexUnit, 0);  // diffuse texture to unit 0
      glUniform1i(specularTexUnit, 1); // specular texture to unit 1
      glUniform1f(shininessLoc, meshComp.mesh->getShininess());

      meshComp.mesh->draw();
    }
  }
}
