#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>
#include <type_traits>
#include <vector>

#include <glm/glm.hpp>

struct VolumeMetadata
{
  glm::ivec3 dimensions{0, 0, 0};
  glm::vec3 spacing{1.0f, 1.0f, 1.0f};
};

template <typename TVoxel>
class VolumeData
{
public:
  VolumeData()
  {
    static_assert(std::is_trivially_copyable_v<TVoxel>, "Volume voxels must be trivially copyable.");
  }

  explicit VolumeData(const std::string& filePath);

  VolumeData(int width, int height, int depth, const glm::vec3& spacing = glm::vec3(1.0f))
    : VolumeData()
  {
    Resize(width, height, depth);
    metadata.spacing = spacing;
  }

  void Resize(int width, int height, int depth)
  {
    if (width <= 0 || height <= 0 || depth <= 0)
    {
      throw std::invalid_argument("Volume dimensions must be positive.");
    }

    metadata.dimensions = glm::ivec3(width, height, depth);
    voxels.resize(static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(depth));
  }

  int GetWidth() const { return metadata.dimensions.x; }
  int GetHeight() const { return metadata.dimensions.y; }
  int GetDepth() const { return metadata.dimensions.z; }
  size_t GetVoxelCount() const { return voxels.size(); }

  const VolumeMetadata& GetMetadata() const { return metadata; }
  void SetSpacing(const glm::vec3& spacing) { metadata.spacing = spacing; }

  TVoxel& At(int x, int y, int z)
  {
    return voxels.at(GetFlatIndex(x, y, z));
  }

  const TVoxel& At(int x, int y, int z) const
  {
    return voxels.at(GetFlatIndex(x, y, z));
  }

  std::vector<TVoxel>& GetVoxels() { return voxels; }
  const std::vector<TVoxel>& GetVoxels() const { return voxels; }

private:
  size_t GetFlatIndex(int x, int y, int z) const
  {
    if (x < 0 || y < 0 || z < 0 ||
        x >= metadata.dimensions.x ||
        y >= metadata.dimensions.y ||
        z >= metadata.dimensions.z)
    {
      throw std::out_of_range("VolumeData index is out of bounds.");
    }

    return static_cast<size_t>(z) * static_cast<size_t>(metadata.dimensions.y) * static_cast<size_t>(metadata.dimensions.x)
      + static_cast<size_t>(y) * static_cast<size_t>(metadata.dimensions.x)
      + static_cast<size_t>(x);
  }

  VolumeMetadata metadata;
  std::vector<TVoxel> voxels;
};
