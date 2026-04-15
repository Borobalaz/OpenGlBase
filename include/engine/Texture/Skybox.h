#pragma once

#include <memory>

#include "Camera/Camera.h"
#include "Geometry/CubeGeometry.h"
#include "Shader.h"
#include "Texture/TextureCube.h"

class Skybox
{
public:
  explicit Skybox(std::shared_ptr<TextureCube> cubemap);

  void Draw(const Camera& camera) const;
  bool IsValid() const;

private:
  std::shared_ptr<TextureCube> cubemap;
  std::shared_ptr<CubeGeometry> geometry;
  std::shared_ptr<Shader> shader;
};