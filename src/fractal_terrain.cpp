#include "fractal_terrain.h"
#include "vertexBuffer.h"
#include <glm/common.hpp>
#include <iostream>

std::vector<Vertex> FractalTerrain::generateTerrain(int subDivCount,
                                                    const Vertex &v1,
                                                    const Vertex &v2,
                                                    const Vertex &v3) {
  // We want to linearaly interpolate on each point to get a new point halfway
  // repeate subDivCount times

  // init points
  edges.clear();
  edges.push_back({v1, v2});
  edges.push_back({v2, v3});
  edges.push_back({v3, v1});

  faces.clear();
  faces.push_back({v1, v2, v3});

  generateSubDivEdges(subDivCount);
  std::vector<Vertex> verts;

  // flatten faces into edges
  for (const std::tuple<Vertex, Vertex, Vertex> &f : faces) {
    verts.push_back(std::get<0>(f));
    verts.push_back(std::get<1>(f));
    verts.push_back(std::get<2>(f));

    // std::cerr << std::get<0>(f).position.x << " " <<
    // std::get<0>(f).position.y
    //           << " " << std::get<0>(f).position.z << "\n";
    // std::cerr << std::get<1>(f).position.x << " " <<
    // std::get<1>(f).position.y
    //           << " " << std::get<1>(f).position.z << "\n";
    // std::cerr << std::get<2>(f).position.x << " " <<
    // std::get<2>(f).position.y
    //           << " " << std::get<2>(f).position.z << "\n";
  }

  return verts;
}

Vertex midVertex(Vertex &v1, Vertex &v2) {
  glm::vec3 pos = glm::mix(v1.position, v2.position, 0.5f);
  glm::vec2 texCoord = glm::mix(v1.texCoord, v2.texCoord, 0.5f);
  glm::vec3 normal = glm::mix(v1.normal, v2.normal, 0.5f);

  return {pos, texCoord, normal};
}

void FractalTerrain::generateSubDivEdges(int subDivCount) {
  if (subDivCount == 0) {
    return;
  }

  // for all faces
  int faceCount = faces.size();
  for (int i = 0; i < faceCount; i++) {
    auto A = std::get<0>(faces[i]);
    auto B = std::get<1>(faces[i]);
    auto C = std::get<2>(faces[i]);

    // add noise to each vertex
    A.position += glm::vec3(0, ((rand() % 10) - 5) / 100.f, 0);
    B.position += glm::vec3(0, ((rand() % 10) - 5) / 100.f, 0);
    C.position += glm::vec3(0, ((rand() % 10) - 5) / 100.f, 0);

    // Compute midpoints for THIS faces 3 edges
    Vertex midAB = midVertex(A, B);
    Vertex midBC = midVertex(B, C);
    Vertex midCA = midVertex(C, A);

    // 4 new faces
    faces.push_back({A, midAB, midCA});
    faces.push_back({B, midBC, midAB});
    faces.push_back({C, midCA, midBC});
    faces.push_back({midAB, midBC, midCA});
  }

  // remove stale faces
  faces.erase(faces.begin(), faces.begin() + faceCount);

  return FractalTerrain::generateSubDivEdges(subDivCount - 1);
}
