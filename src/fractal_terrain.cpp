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

void FractalTerrain::generateSubDivEdges(int subDivCount) {
  if (subDivCount == 0) {
    return;
  }

  // Copy old vertices
  std::vector<Vertex> newVerts;

  // for all edges
  int edgeCount = edges.size();
  for (int i = 0; i < edgeCount; i++) {

    glm::vec3 randNoise = {0, ((rand() % 10) - 5) / 10.f, 0};
    std::cerr << randNoise.y << std::endl;

    // Create and store new vertex
    glm::vec3 pos =
        glm::mix(edges[i].first.position, edges[i].second.position, 0.5f) +
        randNoise;
    glm::vec2 texCoord =
        glm::mix(edges[i].first.texCoord, edges[i].second.texCoord, 0.5f);
    glm::vec3 normal =
        glm::mix(edges[i].first.normal, edges[i].second.normal, 0.5f);

    newVerts.push_back({pos, texCoord, normal});
  }

  // Create and store new edges(will be interior)
  std::vector<std::pair<Vertex, Vertex>> interiorEdges;
  int newVertsCount = newVerts.size();
  for (int i = 0; i < newVertsCount; i++) {
    for (int j = i + 1; j < newVertsCount; j++) {
      interiorEdges.push_back({newVerts[i], newVerts[j]});
      edges.push_back({newVerts[i], newVerts[j]});
    }
  }

  // for all faces
  int faceCount = faces.size();
  std::vector<std::tuple<Vertex, Vertex, Vertex>> newFaces;
  for (int i = 0; i < faceCount; i++) {
    // go throgh each vert in the face
    auto oldV1 = std::get<0>(faces[i]);
    auto oldV2 = std::get<1>(faces[i]);
    auto oldV3 = std::get<2>(faces[i]);

    // connnect oldv1 to edges 1
    std::tuple<Vertex, Vertex, Vertex> newFace1 = {
        oldV1, interiorEdges[1].first, interiorEdges[1].second};
    // connnect oldv2 to edges 0
    std::tuple<Vertex, Vertex, Vertex> newFace2 = {
        oldV2, interiorEdges[0].first, interiorEdges[0].second};

    // connnect oldv3 to edges 2
    std::tuple<Vertex, Vertex, Vertex> newFace3 = {
        oldV3, interiorEdges[2].first, interiorEdges[2].second};

    // connect all edges for the middle
    std::tuple<Vertex, Vertex, Vertex> newFace4 = {interiorEdges[0].first,
                                                   interiorEdges[0].second,
                                                   interiorEdges[1].second};

    faces.push_back(newFace1);
    faces.push_back(newFace2);
    faces.push_back(newFace3);
    faces.push_back(newFace4);
  }
  // remove old faces
  for (int i = 0; i < faceCount; i++) {
    faces.erase(faces.begin());
  }

  return FractalTerrain::generateSubDivEdges(subDivCount - 1);
}
