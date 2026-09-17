#pragma once
#include "Renderer.h"

class DeferredRenderer: public Renderer
{
public:
  DeferredRenderer();

  const RendererDescriptor& GetDescriptor() const override;
  void Draw(const RenderFrame& frame) override;

private:
  RendererDescriptor descriptor;
  unsigned int gBuffer = 1;
};