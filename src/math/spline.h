#pragma once
#include <algorithm>
#include <glm/glm.hpp>
#include <vector>

namespace spline {

// Evaluate Catmull-Rom spline at parameter t for 4 control points
// t in [0, 1], curve passes through p1 (at t=0) and p2 (at t=1)
inline glm::vec3 catmullRom(const glm::vec3 &p0, const glm::vec3 &p1,
                            const glm::vec3 &p2, const glm::vec3 &p3, float t) {
  float t2 = t * t;
  float t3 = t2 * t;
  return 0.5f * ((2.0f * p1) + (-p0 + p2) * t +
                 (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t2 +
                 (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t3);
}

// Subdivide a set of control points into `totalSamples` smooth points.
// If cyclic is true, the spline wraps around (first/last point should match).
inline std::vector<glm::vec3> subdivide(const std::vector<glm::vec3> &points,
                                        int totalSamples, bool cyclic = false) {
  std::vector<glm::vec3> result;
  result.reserve(totalSamples);

  if (points.size() < 2)
    return points;

  int numSegments = (int)points.size() - 1;

  for (int i = 0; i < totalSamples; ++i) {
    float globalT = (float)i / (float)totalSamples * (float)numSegments;
    int seg = (int)globalT;
    float localT = globalT - (float)seg;

    // Clamp segment index
    if (seg >= numSegments) {
      seg = numSegments - 1;
      localT = 1.0f;
    }

    // Get the 4 control points for this segment
    glm::vec3 p0, p1, p2, p3;
    if (cyclic) {
      int n = numSegments; // number of unique points (last == first)
      p0 = points[((seg - 1) % n + n) % n];
      p1 = points[seg % n];
      p2 = points[(seg + 1) % n];
      p3 = points[(seg + 2) % n];
    } else {
      int n = (int)points.size();
      p0 = points[std::max(seg - 1, 0)];
      p1 = points[seg];
      p2 = points[std::min(seg + 1, n - 1)];
      p3 = points[std::min(seg + 2, n - 1)];
    }

    result.push_back(catmullRom(p0, p1, p2, p3, localT));
  }

  // For cyclic paths, close the loop
  if (cyclic) {
    result.push_back(result.front());
  }

  return result;
}

} // namespace spline
