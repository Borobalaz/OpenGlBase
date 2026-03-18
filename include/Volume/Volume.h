#pragma once

#include <memory>
#include <string>

#include <glm/glm.hpp>

#include "IDrawable.h"
#include "Shader.h"
#include "UniformProvider.h"
#include "VolumeData.h"
#include "VolumeGeometry.h"

class VolumeTextureSet;

class Volume : public UniformProvider, public IDrawable
{
public:
  virtual ~Volume() = default;

  void Apply(Shader& shader) const override;
  void Draw(const UniformProvider& frameUniforms) const override;
  bool IsValid() const;

  glm::vec3 position{0.0f, 0.0f, 0.0f};
  glm::vec3 rotation{0.0f, 0.0f, 0.0f};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};

  float stepSize = 0.01f;
  float opacityScale = 0.35f;
  float intensityScale = 1.0f;
  float threshold = 0.05f;
  int maxSteps = 256;

protected:
  Volume(const VolumeMetadata& metadata,
         std::shared_ptr<Shader> shader);

  virtual const VolumeTextureSet& GetTextureSet() const = 0;

  glm::mat4 BuildModelMatrix() const;

  glm::ivec3 dimensions{0, 0, 0};
  glm::vec3 spacing{1.0f, 1.0f, 1.0f};

  std::shared_ptr<VolumeGeometry> geometry;
  std::shared_ptr<Shader> shader;
};
