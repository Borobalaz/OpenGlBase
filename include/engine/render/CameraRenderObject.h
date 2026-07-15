#pragma once

#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>

struct CameraRenderObject
{
  glm::mat4 projMatrix{1.0f};
  glm::mat4 viewMatrix{1.0f};
  glm::vec3 position{0.0f};
};
