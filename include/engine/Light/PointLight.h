#pragma once

#include <glm/glm.hpp>

#include "Light/Light.h"

class PointLight : public Light
{
public:
  PointLight(const std::string& id,
             const glm::vec3& position,
             const glm::vec3& ambient,
             const glm::vec3& diffuse,
             const glm::vec3& specular,
             float constant,
             float linear,
             float quadratic);

  void Apply(Shader& shader) const override;

  glm::vec3 position;
  float constant;
  float linear;
  float quadratic;
};
