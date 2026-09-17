#include "RenderCore/LegacyRenderProxyExtractor.h"

bool LegacyRenderProxyExtractor::Supports(const RendererDescriptor& renderer) const
{
  (void)renderer;
  return true;
}

void LegacyRenderProxyExtractor::Extract(const SceneSnapshot& scene,
                                         RenderExtractionContext& context,
                                         RenderDataStore& output) const
{
  (void)context;
  const auto& proxies = scene.GetRenderProxies();
  auto& channel = output.GetOrCreate<RenderProxy>();
  for (const RenderProxy& proxy : proxies)
  {
    channel.Append(proxy);
  }
}
