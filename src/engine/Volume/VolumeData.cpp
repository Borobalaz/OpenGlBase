#include "VolumeData.h"

#include <utility>

#include "VolumeFileLoader.h"

namespace
{
  template <typename TVoxel>
  VolumeData<TVoxel> LoadOrThrowVolumeData(const std::string& filePath)
  {
    const std::optional<VolumeData<TVoxel>> loaded = VolumeFileLoader::LoadTyped<TVoxel>(filePath);
    if (!loaded.has_value())
    {
      throw std::invalid_argument("Failed to load volume data from file: " + filePath);
    }

    return *loaded;
  }
}

template <>
VolumeData<uint8_t>::VolumeData(const std::string& filePath)
  : VolumeData(LoadOrThrowVolumeData<uint8_t>(filePath))
{
}

template <>
VolumeData<uint16_t>::VolumeData(const std::string& filePath)
  : VolumeData(LoadOrThrowVolumeData<uint16_t>(filePath))
{
}

template <>
VolumeData<float>::VolumeData(const std::string& filePath)
  : VolumeData(LoadOrThrowVolumeData<float>(filePath))
{
}

template <>
VolumeData<glm::mat3>::VolumeData(const std::string& filePath)
  : VolumeData(LoadOrThrowVolumeData<glm::mat3>(filePath))
{
}
