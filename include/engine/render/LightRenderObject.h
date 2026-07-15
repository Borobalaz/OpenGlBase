#pragma once

#include <glm/vec3.hpp>

struct LightRenderObject
{
  enum class Type { Directional, Point, Spot };
  Type type = Type::Directional;
  glm::vec3 color{1.0f, 1.0f, 1.0f};
  float intensity = 1.0f;
  glm::vec3 position{0.0f};
  glm::vec3 direction{0.0f, -1.0f, 0.0f};
};
