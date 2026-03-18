#include "UInt8Volume.h"

#include "Texture3D.h"

UInt8Volume::UInt8Volume(const VolumeData<uint8_t>& volumeData,
                         std::shared_ptr<Shader> shader)
  : Volume(volumeData.GetMetadata(), std::move(shader))
{
  const VolumeMetadata& metadata = volumeData.GetMetadata();
  textureSet.AddTexture(std::make_shared<Texture3D>(
    metadata.dimensions.x,
    metadata.dimensions.y,
    metadata.dimensions.z,
    GL_R8,
    GL_RED,
    GL_UNSIGNED_BYTE,
    volumeData.GetVoxels().data(),
    true));
}

const VolumeTextureSet& UInt8Volume::GetTextureSet() const
{
  return textureSet;
}