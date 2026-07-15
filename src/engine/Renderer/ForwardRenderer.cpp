#include "Renderer/ForwardRenderer.h"

#include <vector>

#include <glad/glad.h>

#include "Geometry/Geometry.h"
#include "Renderer/RenderProxy.h"
#include "Shader.h"

ForwardRenderer::ForwardRenderer()
  : descriptor{
      typeId<ForwardRenderer>(),
      "Forward Renderer",
      CapabilitySet{RenderCapabilities::Rasterization, RenderCapabilities::ForwardShading}}
{
}

const RendererDescriptor& ForwardRenderer::GetDescriptor() const
{
  return descriptor;
}

void ForwardRenderer::Draw(const RenderFrame& frame)
{
  const std::vector<RenderProxy>& renderProxies = frame.data.Read<RenderProxy>();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (const RenderProxy& proxy : renderProxies)
  {
    if (!proxy.visible)
    {
      continue;
    }

    if (proxy.customDraw)
    {
      proxy.customDraw();
      continue;
    }

    Shader* shader = proxy.preferredShader;
    Geometry* geometry = proxy.geometry;
    if (shader == nullptr || geometry == nullptr)
    {
      continue;
    }

    shader->Use();
    proxy.frameUniforms.Apply(*shader);
    shader->Apply(*shader);
    geometry->Draw(*shader);
  }
}
