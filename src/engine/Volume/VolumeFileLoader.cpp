#include "VolumeFileLoader.h"

#include <cstring>
#include <fstream>

namespace
{
  std::optional<VolumeFileHeader> TryReadHeader(const std::string& filePath)
  {
    std::ifstream input(filePath, std::ios::binary);
    if (!input.is_open())
    {
      return std::nullopt;
    }

    VolumeFileHeader header{};
    input.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (!input.good())
    {
      return std::nullopt;
    }

    if (std::memcmp(header.magic, "VXA1", 4) != 0 || header.version != 1)
    {
      return std::nullopt;
    }

    return header;
  }
}

std::optional<LoadedVolumeVariant> VolumeFileLoader::Load(const std::string& filePath)
{
  const std::optional<VolumeFileHeader> header = TryReadHeader(filePath);
  if (!header.has_value())
  {
    return std::nullopt;
  }

  if (const auto matrixF32 = LoadTyped<glm::mat3>(filePath))
  {
    return LoadedVolumeVariant(*matrixF32);
  }

  if (const auto scalarU8 = LoadTyped<uint8_t>(filePath))
  {
    return LoadedVolumeVariant(*scalarU8);
  }

  if (const auto scalarU16 = LoadTyped<uint16_t>(filePath))
  {
    return LoadedVolumeVariant(*scalarU16);
  }

  if (const auto scalarF32 = LoadTyped<float>(filePath))
  {
    return LoadedVolumeVariant(*scalarF32);
  }

  return std::nullopt;
}
