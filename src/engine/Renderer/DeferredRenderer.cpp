#include "Renderer/DeferredRenderer.h"

#include "Renderer/ForwardRenderer.h"

void DeferredRenderer::Render(Scene& scene)
{
  // Legacy compatibility path: the project now uses the forward renderer by default.
  ForwardRenderer forwardRenderer;
  forwardRenderer.Render(scene);
}