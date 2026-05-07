#include "Renderer/ForwardRenderer.h"

#include <vector>

#include <glad/glad.h>

#include "Geometry/Geometry.h"
#include "Shader.h"
#include "Scene/Scene.h"

void ForwardRenderer::Render(Scene& scene)
{
  const std::vector<RenderProxy> renderProxies = scene.GetRenderProxies();

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
