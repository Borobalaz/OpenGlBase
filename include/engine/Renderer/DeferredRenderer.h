#pragma once
#include "Renderer.h"
#include "Shader.h"

class Scene;

class DeferredRenderer: public Renderer
{
public:
  DeferredRenderer() = default;
  void Render(Scene& scene) override;

private:
  unsigned int gBuffer = 1;
};