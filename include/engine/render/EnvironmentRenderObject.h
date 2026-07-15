#pragma once

#include <glm/vec3.hpp>

struct EnvironmentRenderObject
{
  int skyboxTextureId = -1;
  glm::vec3 ambientColor{0.03f, 0.03f, 0.03f};
  float exposure = 1.0f;
};
