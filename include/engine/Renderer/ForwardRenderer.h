#pragma once

#include "Renderer.h"

class ForwardRenderer : public Renderer
{
public:
  ForwardRenderer();

  const RendererDescriptor& GetDescriptor() const override;
  void Draw(const RenderFrame& frame) override;

private:
  RendererDescriptor descriptor;
};
