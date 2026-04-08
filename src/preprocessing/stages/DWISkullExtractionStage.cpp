#include "Preprocessing/stages/DWISkullExtractionStage.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <vector>

#include "Preprocessing/MriPreprocessingStages.h"
#include "VolumeFileLoader.h"

namespace
{
  inline size_t FlatIndex(int x, int y, int z, int width, int height)
  {
    return static_cast<size_t>(z) * static_cast<size_t>(height) * static_cast<size_t>(width) +
           static_cast<size_t>(y) * static_cast<size_t>(width) +
           static_cast<size_t>(x);
  }

  float ComputePercentile(const std::vector<float> &values, float percentile01)
  {
    std::vector<float> finiteValues;
    finiteValues.reserve(values.size());
    for (const float v : values)
    {
      if (std::isfinite(v))
      {
        finiteValues.push_back(v);
      }
    }

    if (finiteValues.empty())
    {
      return 0.0f;
    }

    std::sort(finiteValues.begin(), finiteValues.end());
    const float clampedPercentile = std::clamp(percentile01, 0.0f, 1.0f);
    const size_t n = finiteValues.size();
    const size_t index = static_cast<size_t>(std::round(clampedPercentile * static_cast<float>(n - 1)));
    return finiteValues[index];
  }
}

const char *DWISkullExtractionStage::Name() const
{
  return "DWI skull extraction";
}

void DWISkullExtractionStage::Execute(MriPreprocessingContext &context) const
{
  const auto dwiSeries = VolumeFileLoader::LoadSeries(context.selectedDwiVolumePath);
  
  const int frameCount = dwiSeries->GetFrameCount();
  if (frameCount <= 0)
  {
    throw std::runtime_error("Skull extraction received empty DWI frame set.");
  }

  const VolumeSeriesMetadata &metadata = dwiSeries->GetMetadata();
  const int width = metadata.dimensions.x;
  const int height = metadata.dimensions.y;
  const int depth = metadata.dimensions.z;
  const size_t spatialVoxelCount =
      static_cast<size_t>(width) * static_cast<size_t>(height) * static_cast<size_t>(depth);

  const std::vector<float> &allSignals = dwiSeries->GetVoxels();
  if (allSignals.size() != spatialVoxelCount * static_cast<size_t>(frameCount))
  {
    throw std::runtime_error("Skull extraction signal size mismatch.");
  }

  std::vector<int> b0FrameIndices;
  b0FrameIndices.reserve(context.bValues.size());
  for (size_t i = 0; i < context.bValues.size(); ++i)
  {
    if (context.bValues[i] <= 50.0f)
    {
      b0FrameIndices.push_back(static_cast<int>(i));
    }
  }

  if (b0FrameIndices.empty())
  {
    throw std::runtime_error("Skull extraction requires at least one b0 frame.");
  }

  std::vector<float> s0(spatialVoxelCount, 0.0f);
  for (int z = 0; z < depth; ++z)
  {
    for (int y = 0; y < height; ++y)
    {
      for (int x = 0; x < width; ++x)
      {
        const size_t voxelIndex = FlatIndex(x, y, z, width, height);

        float sum = 0.0f;
        for (int frameIndex : b0FrameIndices)
        {
          const size_t frameOffset = static_cast<size_t>(frameIndex) * spatialVoxelCount;
          sum += allSignals[frameOffset + voxelIndex];
        }

        const float meanS0 = sum / static_cast<float>(b0FrameIndices.size());
        s0[voxelIndex] = meanS0;
      }
    }
  }

  constexpr float kLowPercentile = 0.10f;
  constexpr float kHighPercentile = 0.98f;
  const float lowThreshold = ComputePercentile(s0, kLowPercentile);
  const float highThreshold = ComputePercentile(s0, kHighPercentile);

  if (!(highThreshold > lowThreshold))
  {
    throw std::runtime_error("Skull extraction percentile thresholds are invalid.");
  }

  VolumeData mask(width, height, depth, metadata.spacing);
  std::vector<float> &maskVoxels = mask.GetVoxels();
  for (size_t i = 0; i < spatialVoxelCount; ++i)
  {
    const bool mask1 = s0[i] > lowThreshold;
    const bool mask2 = s0[i] < highThreshold;
    maskVoxels[i] = (mask1 && mask2) ? 1.0f : 0.0f;
  }

  context.outputChannels.Mask = std::move(mask);
}

std::unique_ptr<IMriPreprocessingStage> CreateDwiSkullExtractionStage()
{
  return std::make_unique<DWISkullExtractionStage>();
}
