#include "core/DtiVolumeScene.h"
#include "Shader.h"
#include "Texture/Skybox.h"
#include "Light/PointLight.h"
#include "Light/DirectionalLight.h"
#include "Camera/PerspectiveCamera.h"
#include "Material.h"
#include "Mesh.h"
#include "GameObject.h"

#include <iostream>
#include <glm/glm.hpp>

#include "ui/widgets/inspect_fields/InspectCheckboxFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectNumberFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectFilePickerWidget.h"
#include "ui/widgets/inspect_fields/InspectActionFieldWidget.h"

/**
 * @brief Construct a new Dti Volume Scene:: Dti Volume Scene object
 * 
 */
DtiVolumeScene::DtiVolumeScene()
    : dtiVolume(nullptr)
{
  tractographySettingsInspectable = std::make_shared<MriTractographySettings>();
  currentRequest.tractographySettings = tractographySettingsInspectable;
}

/**
 * @brief Load a DTI dataset and initialize the scene for visualization
 *
 * @param dwiVolumePath The path to the DWI volume
 * @param bvalPath The path to the b-values file
 * @param bvecPath The path to the b-vectors file
 * @return true if the dataset was loaded successfully, false otherwise
 */
bool DtiVolumeScene::LoadDataset(
    const std::string &dwiVolumePath,
    const std::string &bvalPath,
    const std::string &bvecPath)
{
  currentRequest.tractographySettings = tractographySettingsInspectable;
  currentRequest.dwiVolumePath = dwiVolumePath;
  currentRequest.bvalPath = bvalPath;
  currentRequest.bvecPath = bvecPath;

  return ReloadDataset();
}

bool DtiVolumeScene::ReloadDataset()
{
  lastLoadError.clear();

  try
  {
    // Run preprocessing pipeline
    MriPreprocessingResult result = preprocessor.Process(currentRequest);

    // Check if preprocessing succeeded
    if (result.report.sourceVolumePath.empty())
    {
      lastLoadError = "No suitable volume found in dataset";
      return false;
    }

    if (!ApplyPreprocessingResult(result))
    {
      return false;
    }

    // Print preprocessing report
    std::cout << "Loaded: " << result.report.sourceVolumePath << std::endl;
    std::cout << "Executed stages:\n";
    for (const auto &stage : result.report.executedStages)
    {
      std::cout << "    - " << stage << "\n";
    }

    if (!result.report.warnings.empty())
    {
      std::cout << "  Warnings:\n";
      for (const auto &warning : result.report.warnings)
      {
        std::cout << "    [!] " << warning << "\n";
      }
    }

    return true;
  }
  catch (const std::exception &ex)
  {
    lastLoadError = std::string("Exception during preprocessing: ") + ex.what();
    std::cerr << lastLoadError << std::endl;
    return false;
  }
  catch (...)
  {
    lastLoadError = "Unknown error during preprocessing";
    std::cerr << lastLoadError << std::endl;
    return false;
  }
}

void DtiVolumeScene::ClearProcessedScene()
{
  dtiVolume.reset();
  brainSurfaceObject.reset();
  streamlineObject.reset();
  ClearGameObjects();
  ClearVolumes();
}

bool DtiVolumeScene::ApplyPreprocessingResult(const MriPreprocessingResult &result)
{
  const int previousRenderMode = dtiVolume ? dtiVolume->GetSelectedRenderModeIndex() : 0;
  const int previousChannel = dtiVolume ? dtiVolume->GetSelectedChannelIndex() : 0;

  ClearProcessedScene();
  RebuildInspectProviders();

  // Create DTI volume from processed channels with tensor-eigenvector shader
  std::shared_ptr<Shader> volumeShader = std::make_shared<Shader>(
      "dti_volume_main_shader",
      "shaders/volume_vertex.glsl",
      "shaders/dti_fragment_shaders/volume_dti_tensor_fragment.glsl");
  (*volumeShader)["shader.sliceZ"] = 0.5f;
  (*volumeShader)["shader.density"] = 1.0f;

  dtiVolume = std::make_shared<DTIVolume>("dti_volume_main", result.channels, volumeShader);
  dtiVolume->SetRotation(glm::vec3(-90.0f / 180.0f * glm::pi<float>(), 0.0f, 0.0f));
  dtiVolume->SetSelectedRenderModeIndex(previousRenderMode);
  dtiVolume->SetSelectedChannelIndex(previousChannel);

  AddVolume(dtiVolume);
  AddInspectProvider(dtiVolume);

  // Register shaders for hot reload tracking
  dtiVolume->RegisterShadersWithScene(this);

  // FA surface mesh
  if (result.surfaceMesh)
  {
    std::shared_ptr<Shader> meshShader = std::make_shared<Shader>(
        "dti_brain_surface_shader",
        "shaders/vertex.glsl",
        "shaders/fragment.glsl");
    this->RegisterShader("dti_brain_surface_shader", meshShader);

    std::shared_ptr<Material> meshMaterial = std::make_shared<Material>(meshShader);
    meshMaterial->SetAmbientColor(glm::vec3(0.35f, 0.35f, 0.35f));
    meshMaterial->SetDiffuseColor(glm::vec3(0.62f, 0.62f, 0.62f));
    meshMaterial->SetSpecularColor(glm::vec3(0.08f, 0.08f, 0.08f));
    meshMaterial->SetShininess(18.0f);

    result.surfaceMesh->SetMaterial(meshMaterial);
    brainSurfaceObject = std::make_shared<GameObject>("dti_brain_surface");
    brainSurfaceObject->AddMesh(result.surfaceMesh);
    brainSurfaceObject->SetRotation(glm::vec3(-90.0f / 180.0f * glm::pi<float>(), 0.0f, 0.0f));
    AddGameObject(brainSurfaceObject);
    AddInspectProvider(brainSurfaceObject);
  }

  if (result.streamlineMesh)
  {
    std::shared_ptr<Shader> streamlineShader = std::make_shared<Shader>(
        "dti_streamline_shader",
        "shaders/streamline_vertex.glsl",
        "shaders/streamline_fragment.glsl");

    this->RegisterShader("dti_streamline_shader", streamlineShader);
    
    std::shared_ptr<Material> streamlineMaterial = std::make_shared<Material>(streamlineShader);
    streamlineMaterial->SetAmbientColor(glm::vec3(1.0f, 0.58f, 0.16f));
    streamlineMaterial->SetDiffuseColor(glm::vec3(1.0f, 0.58f, 0.16f));
    streamlineMaterial->SetSpecularColor(glm::vec3(0.0f, 0.0f, 0.0f));
    streamlineMaterial->SetShininess(1.0f);

    result.streamlineMesh->SetMaterial(streamlineMaterial);
    streamlineObject = std::make_shared<GameObject>("dti_streamlines");
    streamlineObject->AddMesh(result.streamlineMesh);
    streamlineObject->SetRotation(glm::vec3(-90.0f / 180.0f * glm::pi<float>(), 0.0f, 0.0f));
    AddGameObject(streamlineObject);
    AddInspectProvider(streamlineObject);
  }

  return true;
}

/**
 * @brief Update the scene, 
 *  applying a slow rotation to the brain surface and streamlines for better visualization of the 3D structure.
 * 
 * @param deltaTime 
 */
void DtiVolumeScene::Update(float deltaTime)
{
  Scene::Update(deltaTime);

  if (rotationEnabled)
  {
    streamlineObject->SetRotation(streamlineObject->GetRotation() + glm::vec3(0.0f, 0.0f, deltaTime * rotationSpeed));
    brainSurfaceObject->SetRotation(brainSurfaceObject->GetRotation() + glm::vec3(0.0f, 0.0f, deltaTime * rotationSpeed));
    dtiVolume->SetRotation(dtiVolume->GetRotation() + glm::vec3(0.0f, 0.0f, deltaTime * rotationSpeed));
  }
}

/**
 * @brief Create inspect widgets for controlling scene parameters such as rotation.
 * 
 * @return std::vector<std::shared_ptr<IInspectWidget>> 
 */
std::vector<std::shared_ptr<IInspectWidget>> DtiVolumeScene::GetInspectFields()
{
  // Basic scene properties
  std::vector<std::shared_ptr<IInspectWidget>> fields = Scene::GetInspectFields(); // Get base scene fields

  auto dwiField = std::make_shared<InspectFilePickerWidget>(
      "dwiVolumePath",
      "DWI Volume",
      "Preprocessing",
      "Select DWI volume",
      "DWI volume (*.nii *.nii.gz *.nrrd *.mha *.mhd *.vxa);;All files (*.*)");
  dwiField->SetValue(QString::fromStdString(currentRequest.dwiVolumePath));
  dwiField->valueChangedCallback = [this](const QVariant &value)
  {
    currentRequest.dwiVolumePath = value.toString().toStdString();
  };
  fields.push_back(dwiField);

  auto bvalField = std::make_shared<InspectFilePickerWidget>(
      "bvalPath",
      "B-Values",
      "Preprocessing",
      "Select b-values file",
      "B-values (*.bval *.txt *.csv);;All files (*.*)");
  bvalField->SetValue(QString::fromStdString(currentRequest.bvalPath));
  bvalField->valueChangedCallback = [this](const QVariant &value)
  {
    currentRequest.bvalPath = value.toString().toStdString();
  };
  fields.push_back(bvalField);

  auto bvecField = std::make_shared<InspectFilePickerWidget>(
      "bvecPath",
      "B-Vectors",
      "Preprocessing",
      "Select b-vectors file",
      "B-vectors (*.bvec *.txt *.csv);;All files (*.*)");
  bvecField->SetValue(QString::fromStdString(currentRequest.bvecPath));
  bvecField->valueChangedCallback = [this](const QVariant &value)
  {
    currentRequest.bvecPath = value.toString().toStdString();
  };
  fields.push_back(bvecField);

  const std::vector<std::shared_ptr<IInspectWidget>> tractographyFields = tractographySettingsInspectable ? tractographySettingsInspectable->GetInspectFields() : std::vector<std::shared_ptr<IInspectWidget>>{};
  fields.insert(fields.end(), tractographyFields.begin(), tractographyFields.end());

  auto rerunField = std::make_shared<InspectActionFieldWidget>(
      "rerunPreprocessing",
      "Rerun preprocessing",
      "Preprocessing");
  rerunField->actionCallback = [this]()
  {
    const bool reloaded = reloadCallback ? reloadCallback() : ReloadDataset();
    if (!reloaded)
    {
      std::cout << "DTI dataset reload failed: " << lastLoadError << "\n";
    }
  };
  fields.push_back(rerunField);

  // Rotation controls
  auto rotationEnabledField = std::make_shared<InspectCheckboxFieldWidget>(
      "rotationEnabled",
      "Enabled",
      "Rotation");
  rotationEnabledField->SetValue(QVariant(rotationEnabled));
  rotationEnabledField->valueChangedCallback = [this](const QVariant &value)
  {
    rotationEnabled = value.toBool();
  };
  fields.push_back(rotationEnabledField);

  auto rotationSpeedField = std::make_shared<InspectNumberFieldWidget>(
      "rotationSpeed",
      "Speed",
      "Rotation",
      [this]()
      { return (double)rotationSpeed; },
      [this](double newValue)
      { rotationSpeed = newValue; },
      0.0,
      5.0,
      0.01);
  fields.push_back(rotationSpeedField);

  return fields;
}
