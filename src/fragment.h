#include <glm/glm.hpp>
#include "Color.h"
#pragma once

struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 originalPos;
    bool renderize;
};

struct Fragment {
    glm::ivec2 position;
    Color color; 
    double z;
    float intensity;
    glm::vec3 originalPos; 
};

struct FragColor {
  Color color;
  double z;
};