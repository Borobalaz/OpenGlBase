#pragma once

#include "Volume.h"
#include "VolumeTextureSet.h"

class FloatVolume final : public Volume
{
public:
  explicit FloatVolume(const VolumeData<float>& volumeData,
                       std::shared_ptr<Shader> shader);

private:
  const VolumeTextureSet& GetTextureSet() const override;

  VolumeTextureSet textureSet;
};