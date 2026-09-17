#include "Renderer/DeferredRenderer.h"

#include "Renderer/ForwardRenderer.h"

DeferredRenderer::DeferredRenderer()
  : descriptor{
      typeId<DeferredRenderer>(),
      "Deferred Renderer",
      CapabilitySet{
        RenderCapabilities::Rasterization,
        RenderCapabilities::DeferredShading,
        RenderCapabilities::Compute}}
{
}

const RendererDescriptor& DeferredRenderer::GetDescriptor() const
{
  return descriptor;
}

void DeferredRenderer::Draw(const RenderFrame& frame)
{
  // Legacy compatibility path: the project now uses the forward renderer by default.
  ForwardRenderer forwardRenderer;
  forwardRenderer.Draw(frame);
}