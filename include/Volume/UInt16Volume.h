#pragma once

#include "Volume.h"
#include "VolumeTextureSet.h"

class UInt16Volume final : public Volume
{
public:
  explicit UInt16Volume(const VolumeData<uint16_t>& volumeData,
                        std::shared_ptr<Shader> shader);

private:
  const VolumeTextureSet& GetTextureSet() const override;

  VolumeTextureSet textureSet;
};