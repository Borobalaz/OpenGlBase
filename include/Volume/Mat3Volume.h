#pragma once

#include "Volume.h"
#include "VolumeTextureSet.h"

class Mat3Volume final : public Volume
{
public:
  explicit Mat3Volume(const VolumeData<glm::mat3>& volumeData,
                      std::shared_ptr<Shader> shader);

private:
  const VolumeTextureSet& GetTextureSet() const override;

  VolumeTextureSet textureSet;
};