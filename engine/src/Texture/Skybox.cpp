#include "Texture/Skybox.h"

#include <glm/glm.hpp>

#include "Renderer/RenderProxy.h"
#include "Geometry/Geometry.h"
#include "Shader.h"
#include "Camera/Camera.h"

namespace
{
  constexpr unsigned int kSkyboxTextureUnit = 5;
}

Skybox::Skybox(std::shared_ptr<TextureCube> cubemap)
  : cubemap(std::move(cubemap)),
    geometry(std::make_shared<CubeGeometry>()),
    shader(std::make_shared<Shader>("skybox_shader", "shaders/skybox/skybox_vertex.glsl", "shaders/skybox/skybox_fragment.glsl"))
{
}

void Skybox::Draw(const Camera& camera) const
{
  if (!IsValid())
  {
    return;
  }

  GLboolean previousDepthWriteMask = GL_TRUE;
  GLint previousDepthFunc = GL_LESS;
  GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

  glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthWriteMask);
  glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);

  glDepthMask(GL_FALSE);
  glDepthFunc(GL_LEQUAL);
  glDisable(GL_CULL_FACE);

  shader->Use();
  shader->SetMat4("viewMatrix", glm::mat4(glm::mat3(camera.GetViewMatrix())));
  shader->SetMat4("projectionMatrix", camera.GetProjectionMatrix());
  shader->SetTexture("skyboxTexture", kSkyboxTextureUnit);
  cubemap->Bind(kSkyboxTextureUnit);
  geometry->Draw(*shader);

  if (cullFaceEnabled)
  {
    glEnable(GL_CULL_FACE);
  }

  glDepthMask(previousDepthWriteMask);
  glDepthFunc(previousDepthFunc);
}

bool Skybox::IsValid() const
{
  return cubemap != nullptr && cubemap->IsValid() && shader != nullptr && shader->ID != 0;
}

void Skybox::BuildRenderProxy(RenderProxy& renderProxy) const
{
  renderProxy.visible = IsValid();
  if (!renderProxy.visible)
  {
    return;
  }

  // Use the skybox's shader and geometry for drawing, but perform custom GL state setup
  // via a customDraw lambda so we can disable depth writes and adjust culling.
  std::shared_ptr<Shader> shaderCopy = shader;
  std::shared_ptr<Geometry> geometryCopy = geometry;
  std::shared_ptr<TextureCube> cubemapCopy = cubemap;

  // Capture the frame uniforms by value so the lambda has access to the camera/projection
  // that Scene already placed into renderProxy.frameUniforms before calling this method.
  auto frameUniformsCopy = renderProxy.frameUniforms;

  renderProxy.preferredShader = shaderCopy.get();
  renderProxy.geometry = geometryCopy.get();

  renderProxy.customDraw = [shaderCopy, geometryCopy, cubemapCopy, frameUniformsCopy]() {
    if (!shaderCopy || !geometryCopy || !cubemapCopy)
    {
      return;
    }

    GLboolean previousDepthWriteMask = GL_TRUE;
    GLint previousDepthFunc = GL_LESS;
    GLboolean cullFaceEnabled = glIsEnabled(GL_CULL_FACE);

    glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthWriteMask);
    glGetIntegerv(GL_DEPTH_FUNC, &previousDepthFunc);

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    shaderCopy->Use();
    frameUniformsCopy.Apply(*shaderCopy);
    shaderCopy->Apply(*shaderCopy);
    constexpr unsigned int kSkyboxTextureUnit = 5;
    shaderCopy->SetTexture("skyboxTexture", kSkyboxTextureUnit);
    cubemapCopy->Bind(kSkyboxTextureUnit);
    geometryCopy->Draw(*shaderCopy);

    if (cullFaceEnabled)
    {
      glEnable(GL_CULL_FACE);
    }

    glDepthMask(previousDepthWriteMask);
    glDepthFunc(previousDepthFunc);
  };
}