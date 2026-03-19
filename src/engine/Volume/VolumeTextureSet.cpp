#include "VolumeTextureSet.h"

void VolumeTextureSet::AddTexture(std::shared_ptr<Texture3D> texture)
{
  if (texture)
  {
    textures.push_back(std::move(texture));
  }
}

void VolumeTextureSet::Bind(Shader& shader, unsigned int baseUnit, const std::string& uniformBaseName) const
{
  if (textures.empty())
  {
    return;
  }

  for (size_t i = 0; i < textures.size(); ++i)
  {
    if (!textures[i])
    {
      continue;
    }

    const std::string uniformName = uniformBaseName + "[" + std::to_string(i) + "]";
    if (!shader.HasUniform(uniformName))
    {
      continue;
    }

    textures[i]->Bind(baseUnit + static_cast<unsigned int>(i));
    shader.SetTexture(uniformName, static_cast<int>(baseUnit + static_cast<unsigned int>(i)));
  }
}

bool VolumeTextureSet::IsValid() const
{
  if (textures.empty())
  {
    return false;
  }

  for (const auto& texture : textures)
  {
    if (!texture || !texture->IsValid())
    {
      return false;
    }
  }

  return true;
}

size_t VolumeTextureSet::Size() const
{
  return textures.size();
}
