#pragma once

#include "Volume.h"
#include "VolumeTextureSet.h"

class UInt8Volume final : public Volume
{
public:
  explicit UInt8Volume(const VolumeData<uint8_t>& volumeData,
                       std::shared_ptr<Shader> shader);

private:
  const VolumeTextureSet& GetTextureSet() const override;

  VolumeTextureSet textureSet;
};