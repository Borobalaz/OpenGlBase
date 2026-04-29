#include "Preprocessing/MriTractographySettings.h"

#include <algorithm>
#include <cmath>

#include "ui/widgets/inspect_fields/InspectNumberFieldWidget.h"

std::string MriTractographySettings::GetInspectDisplayName() const
{
  return "Tractography Settings";
}

std::vector<std::shared_ptr<IInspectWidget>> MriTractographySettings::GetInspectFields()
{
  std::vector<std::shared_ptr<IInspectWidget>> fields;

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.faSeedThreshold",
    "FA Seed Threshold",
    "Tractography",
    [this]() { return faSeedThresholdValue; },
    [this](double newValue) { faSeedThresholdValue = static_cast<float>(newValue); },
    0.0,
    1.0,
    0.01));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.faStopThreshold",
    "FA Stop Threshold",
    "Tractography",
    [this]() { return faStopThresholdValue; },
    [this](double newValue) { faStopThresholdValue = static_cast<float>(newValue); },
    0.0,
    1.0,
    0.01));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.l1StopThreshold",
    "L1 Stop Threshold",
    "Tractography",
    [this]() { return l1StopThresholdValue; },
    [this](double newValue) { l1StopThresholdValue = static_cast<float>(newValue); },
    0.0,
    1.0,
    0.000001));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.stepSizeVoxels",
    "Step Size (voxels)",
    "Tractography",
    [this]() { return stepSizeVoxelsValue; },
    [this](double newValue) { stepSizeVoxelsValue = static_cast<float>(newValue); },
    0.001,
    10.0,
    0.01));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.maxStepsPerStreamline",
    "Max Steps per Streamline",
    "Tractography",
    [this]() { return static_cast<double>(maxStepsPerStreamlineValue); },
    [this](double newValue) { maxStepsPerStreamlineValue = static_cast<int>(std::lround(newValue)); },
    1.0,
    10000.0,
    1.0));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.seedStride",
    "Seed Stride",
    "Tractography",
    [this]() { return static_cast<double>(seedStrideValue); },
    [this](double newValue) { seedStrideValue = std::max(1, static_cast<int>(std::lround(newValue))); },
    1.0,
    64.0,
    1.0));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.maxSeeds",
    "Max Seeds",
    "Tractography",
    [this]() { return static_cast<double>(maxSeedsValue); },
    [this](double newValue) { maxSeedsValue = static_cast<size_t>(std::max(1, static_cast<int>(std::lround(newValue)))); },
    1.0,
    100000.0,
    1.0));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.minPointsPerStreamline",
    "Min Points per Streamline",
    "Tractography",
    [this]() { return static_cast<double>(minPointsPerStreamlineValue); },
    [this](double newValue) { minPointsPerStreamlineValue = static_cast<size_t>(std::max(1, static_cast<int>(std::lround(newValue)))); },
    1.0,
    10000.0,
    1.0));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.tubeRadius",
    "Tube Radius",
    "Tractography",
    [this]() { return tubeRadiusValue; },
    [this](double newValue) { tubeRadiusValue = static_cast<float>(newValue); },
    0.00001,
    0.1,
    0.0001));

  fields.push_back(std::make_shared<InspectNumberFieldWidget>(
    "tractography.tubeRadialSegments",
    "Tube Radial Segments",
    "Tractography",
    [this]() { return static_cast<double>(tubeRadialSegmentsValue); },
    [this](double newValue) { tubeRadialSegmentsValue = static_cast<unsigned int>(std::max(3, static_cast<int>(std::lround(newValue)))); },
    3.0,
    32.0,
    1.0));

  return fields;
}
