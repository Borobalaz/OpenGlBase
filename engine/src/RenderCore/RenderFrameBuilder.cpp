#include "RenderCore/RenderFrameBuilder.h"

RenderFrame RenderFrameBuilder::Build(const SceneSnapshot& snapshot,
                                      const RendererDescriptor& renderer,
                                      const ExtractionRegistry& extractionRegistry) const
{
  RenderFrame frame;
  RenderExtractionContext context(renderer);
  extractionRegistry.ExtractFor(renderer, snapshot, context, frame.data);
  return frame;
}
