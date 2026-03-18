#include "Volume.h"

#include <glm/gtc/matrix_transform.hpp>

#include "VolumeTextureSet.h"

namespace
{
  constexpr unsigned int kVolumeTextureBaseUnit = 8;
}

Volume::Volume(const VolumeMetadata& metadata,
               std::shared_ptr<Shader> shader)
  : dimensions(metadata.dimensions),
    spacing(metadata.spacing),
    geometry(std::make_shared<VolumeGeometry>()),
    shader(std::move(shader))
{
}

void Volume::Apply(Shader& shader) const
{
  if (shader.HasUniform("volume.dimensions"))
  {
    shader.SetVec3("volume.dimensions", glm::vec3(dimensions));
  }

  if (shader.HasUniform("volume.spacing"))
  {
    shader.SetVec3("volume.spacing", spacing);
  }

  if (shader.HasUniform("volume.stepSize"))
  {
    shader.SetFloat("volume.stepSize", stepSize);
  }

  if (shader.HasUniform("volume.opacityScale"))
  {
    shader.SetFloat("volume.opacityScale", opacityScale);
  }

  if (shader.HasUniform("volume.intensityScale"))
  {
    shader.SetFloat("volume.intensityScale", intensityScale);
  }

  if (shader.HasUniform("volume.threshold"))
  {
    shader.SetFloat("volume.threshold", threshold);
  }

  if (shader.HasUniform("volume.maxSteps"))
  {
    shader.SetInt("volume.maxSteps", maxSteps);
  }

  if (shader.HasUniform("volume.textureCount"))
  {
    shader.SetInt("volume.textureCount", static_cast<int>(GetTextureSet().Size()));
  }
}

void Volume::Draw(const UniformProvider& frameUniforms) const
{
  if (!IsValid())
  {
    return;
  }

  GLboolean previousBlendEnabled = glIsEnabled(GL_BLEND);
  GLboolean previousDepthWriteMask = GL_TRUE;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &previousDepthWriteMask);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDepthMask(GL_FALSE);

  shader->Use();
  frameUniforms.Apply(*shader);
  Apply(*shader);
  shader->SetMat4("volumeObject.modelMatrix", BuildModelMatrix());
  shader->SetMat4("volumeObject.inverseModelMatrix", glm::inverse(BuildModelMatrix()));
  GetTextureSet().Bind(*shader, kVolumeTextureBaseUnit, "volumeTextures");
  geometry->Draw(*shader);

  if (!previousBlendEnabled)
  {
    glDisable(GL_BLEND);
  }
  glDepthMask(previousDepthWriteMask);
}

bool Volume::IsValid() const
{
  return dimensions.x > 0 && dimensions.y > 0 && dimensions.z > 0 &&
         geometry != nullptr && shader != nullptr && shader->ID != 0 &&
         GetTextureSet().IsValid();
}

glm::mat4 Volume::BuildModelMatrix() const
{
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, scale);
  return model;
}
