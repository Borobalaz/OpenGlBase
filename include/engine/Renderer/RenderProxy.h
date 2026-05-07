#pragma once

#include <functional>

#include "Uniform/CompositeUniformProvider.h"

class Shader;
class Geometry;

struct RenderProxy
{
  CompositeUniformProvider frameUniforms;
  Shader* preferredShader = nullptr;
  Geometry* geometry = nullptr;
  std::function<void()> customDraw;
  bool visible = true;
};
