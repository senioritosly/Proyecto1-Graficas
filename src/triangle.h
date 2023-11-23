#include <glm/glm.hpp>
#include <vector>
#include "fragment.h"
#include "line.h"
#include "settings.h"

#pragma once

glm::vec3 L = glm::vec3(0.0f, 0.0f, 0.0f);

std::pair<float, float> barycentricCoordinates(const glm::ivec2& P, const glm::vec3& A, const glm::vec3& B, const glm::vec3& C) {
    glm::vec3 bary = glm::cross(
        glm::vec3(C.x - A.x, B.x - A.x, A.x - P.x),
        glm::vec3(C.y - A.y, B.y - A.y, A.y - P.y)
    );

    if (abs(bary.z) < 1) {
        return std::make_pair(-1, -1);
    }

    return std::make_pair(
        bary.y / bary.z,
        bary.x / bary.z
    );    
}

std::vector<Fragment> triangle(const Vertex& a, const Vertex& b, const Vertex& c, std::string name, glm::vec3 worldPos) {

  std::vector<Fragment> fragments;

  if (!a.renderize && !b.renderize && !c.renderize)
        return fragments;

  glm::vec3 A = a.position;
  glm::vec3 B = b.position;
  glm::vec3 C = c.position;

  float minX = std::min(std::min(A.x, B.x), C.x);
  float minY = std::min(std::min(A.y, B.y), C.y);
  float maxX = std::max(std::max(A.x, B.x), C.x);
  float maxY = std::max(std::max(A.y, B.y), C.y);

  // Iterate over each point in the bounding box
  for (int y = static_cast<int>(std::ceil(minY)); y <= static_cast<int>(std::floor(maxY)); ++y) {
    for (int x = static_cast<int>(std::ceil(minX)); x <= static_cast<int>(std::floor(maxX)); ++x) {
      if (x < 0 || y < 0 || y > SCREEN_HEIGHT || x > SCREEN_WIDTH)
        continue;
        
      glm::ivec2 P(x, y);
      auto barycentric = barycentricCoordinates(P, A, B, C);
      float w = 1 - barycentric.first - barycentric.second;
      float v = barycentric.first;
      float u = barycentric.second;
      float epsilon = 1e-10;

      if (w < epsilon || v < epsilon || u < epsilon)
        continue;
          
      double z = A.z * w + B.z * v + C.z * u;
      
       glm::vec3 normal = glm::normalize( 
           a.normal * w + b.normal * v + c.normal * u
       );
       

      L = -worldPos;
      L = glm::normalize(L);
      //normal = a.normal; // assume flatness
      float intensity = glm::dot(normal, L);
      
      if (intensity < 0 && name != "sun" && name !="ship")
        intensity = 0;

      glm::vec3 originalPos = a.originalPos * w + b.originalPos * v + c.originalPos * u;

      fragments.push_back(
        Fragment{
          glm::vec3(static_cast<uint16_t>(P.x), static_cast<uint16_t>(P.y), 0),
          Color(205, 205, 205), //color blanco hardcodeado
          z,
          intensity,
          originalPos}
      );
    }
}
  return fragments;
}