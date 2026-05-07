#pragma once

#include "Renderer.h"

class ForwardRenderer : public Renderer
{
public:
  ForwardRenderer() = default;

  void Render(Scene& scene) override;
};
