#pragma once

#include "RenderCore/Extraction.h"

class LegacyRenderProxyExtractor final : public IRenderExtractor
{
public:
  bool Supports(const RendererDescriptor& renderer) const override;

  void Extract(const SceneSnapshot& scene,
               RenderExtractionContext& context,
               RenderDataStore& output) const override;
};
