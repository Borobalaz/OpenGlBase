#include "UInt16Volume.h"

#include "Texture3D.h"

UInt16Volume::UInt16Volume(const VolumeData<uint16_t>& volumeData,
                           std::shared_ptr<Shader> shader)
  : Volume(volumeData.GetMetadata(), std::move(shader))
{
  const VolumeMetadata& metadata = volumeData.GetMetadata();
  textureSet.AddTexture(std::make_shared<Texture3D>(
    metadata.dimensions.x,
    metadata.dimensions.y,
    metadata.dimensions.z,
    GL_R16,
    GL_RED,
    GL_UNSIGNED_SHORT,
    volumeData.GetVoxels().data(),
    true));
}

const VolumeTextureSet& UInt16Volume::GetTextureSet() const
{
  return textureSet;
}