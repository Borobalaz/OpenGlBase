#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct MeshRenderObject
{
  int meshId = -1;      // opaque mesh identifier
  int materialId = -1;  // opaque material identifier
  glm::mat4 transform{1.0f};
  glm::vec3 aabbMin{0.0f};
  glm::vec3 aabbMax{0.0f};
};
