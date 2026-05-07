#include "Scene/Scene.h"
#include <algorithm>
#include <string>

#include "Mesh.h"
#include "Geometry/ModelLoader.h"
#include "Geometry/SphereGeometry.h"
#include "Texture/Skybox.h"
#include "Texture/Texture2D.h"
#include "Renderer/ForwardRenderer.h"
#include "Volume/FloatVolume.h"
#include "Volume/VolumeFileLoader.h"

#include <cmath>
#include <iostream>

#include <glm/gtc/constants.hpp>

#include "ui/widgets/inspect_fields/InspectColorFieldWidget.h"

namespace
{
  constexpr int kMaxLights = 16;
}

/**
 * @brief Construct a new Scene:: Scene object
 * 
 */
Scene::Scene()
  : clearColor{1.0f, 1.0f, 1.0f, 1.0f},
    camera(std::make_shared<PerspectiveCamera>(45.0f, 800.0f / 600.0f, 0.1f, 100.0f))
{
  renderer = std::make_unique<ForwardRenderer>();

  AddInspectProvider(this);
  AddInspectProvider(camera);

  // ------------- SHADERS (PBR) -------------
  std::shared_ptr<Shader> defaultShader = std::make_shared<Shader>(
    "default",
    "shaders/vertex.glsl",
    "shaders/pbr_fragment.glsl"
  );
  // ------------- MATERIALS -------------
  std::shared_ptr<Material> boneMaterial = std::make_shared<Material>(defaultShader);
  boneMaterial->SetAlbedoTextureFromFile("assets/materials/bone/bone_albedo.png");
  boneMaterial->SetNormalTextureFromFile("assets/materials/bone/bone_normal-ogl.png");
  boneMaterial->SetRoughnessTextureFromFile("assets/materials/bone/bone_roughness.png");
  boneMaterial->SetMetallicTextureFromFile("assets/materials/bone/bone_metallic.png");

  std::shared_ptr<Material> donutMaterial = std::make_shared<Material>(defaultShader);
  donutMaterial->SetAlbedoTextureFromFile("assets/materials/donut/Poliigon_FoodPastryDonut_10737_BaseColor.jpg");
  donutMaterial->SetNormalTextureFromFile("assets/materials/donut/Poliigon_FoodPastryDonut_10737_Normal.png");
  donutMaterial->SetRoughnessTextureFromFile("assets/materials/donut/Poliigon_FoodPastryDonut_10737_Roughness.jpg");
  donutMaterial->SetMetallicTextureFromFile("assets/materials/donut/Poliigon_FoodPastryDonut_10737_Metallic.jpg");

  // ------------- GAME OBJECTS -------------
  auto sphere1 = std::make_shared<GameObject>(
    std::make_shared<SphereGeometry>(0.2f, 64, 32),
    boneMaterial,
    "sphere1");

  auto sphere2 = std::make_shared<GameObject>(
    std::make_shared<SphereGeometry>(0.2f, 64, 32),
    donutMaterial,
    "sphere2");
  sphere2->SetPosition(glm::vec3(0.5f, 0.0f, 0.0f));

  //sphere1->SetUpdate([sphere1](float deltaTime)
  //{
  //  const float rotationSpeed = glm::radians(5.0f); // 20 degrees per second
  //  const glm::vec3 currentRotation = sphere1->GetRotation();
  //  sphere1->SetRotation(currentRotation + glm::vec3(0.0f, rotationSpeed * deltaTime, 0.0f));
  //});
  AddGameObject(sphere1);
  AddGameObject(sphere2);

  // ------------- VOLUME -------------

  // ------------- LIGHTS -------------
  std::shared_ptr<DirectionalLight> directionalLight = std::make_shared<DirectionalLight>(
    "dirLight",
    glm::vec3(0.2f, 0.2f, 0.2f),
    glm::vec3(0.5f, 0.5f, 0.5f),
    glm::vec3(1.0f, 1.0f, 1.0f),
    glm::vec3(-1.0f, -1.0f, -1.0f)
  );
  AddLight(directionalLight);
}

/**
 * @brief Initialize the scene, 
 *        enable depth testing, set clear color, set depth function, 
 *        enable seamless cubemap sampling
 * 
 */
void Scene::Init()
{
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

/**
 * @brief Game loop update function
 * 
 * @param deltaTime 
 */
void Scene::Update(float deltaTime)
{
  if (camera)
  {
    camera->Update(deltaTime);
  }
  for (const auto& updateable : updateables)
  {
    if (updateable)
    {
      updateable->Update(deltaTime);
    }
  }

  for (const auto& light : lights)
  {
    if (light)
    {
      light->Update(deltaTime);
    }
  }

  // Clear per-frame input impulses after the update pass consumed them.
  inputState.ResetFrameTransientState();
}

void Scene::Render()
{
  glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

  if (renderer)
  {
    renderer->Render(*this);
  }
}

/**
 * @brief Apply the scene's uniforms to the given shader
 * 
 * @param shader The shader to apply uniforms to
 */
void Scene::Apply(Shader& shader) const
{
  if (!shader.HasUniform("lightCount"))
  {
    return;
  }

  const int enabledCount = static_cast<int>(std::count_if(lights.begin(), lights.end(),
    [](const auto& lightEntry)
    {
      const std::shared_ptr<Light>& light = lightEntry;
      return light && light->GetEnabled();
    }));
  const int lightCount = static_cast<int>(std::min(enabledCount, kMaxLights));
  shader.SetInt("lightCount", lightCount);
}

/**
 * @brief Clear all game objects from the scene
 * 
 */
void Scene::ClearGameObjects()
{
  drawables.erase(std::remove_if(drawables.begin(), drawables.end(),
    [](const std::shared_ptr<IDrawable>& drawable)
    {
      return std::dynamic_pointer_cast<GameObject>(drawable) != nullptr;
    }),
    drawables.end());

  updateables.erase(std::remove_if(updateables.begin(), updateables.end(),
    [](const std::shared_ptr<IUpdateable>& updateable)
    {
      return std::dynamic_pointer_cast<GameObject>(updateable) != nullptr;
    }),
    updateables.end());
}

/**
 * @brief Clear all volumes from the scene
 * 
 */
void Scene::ClearVolumes()
{
  drawables.erase(std::remove_if(drawables.begin(), drawables.end(),
    [](const std::shared_ptr<IDrawable>& drawable)
    {
      return std::dynamic_pointer_cast<Volume>(drawable) != nullptr;
    }),
    drawables.end());

  updateables.erase(std::remove_if(updateables.begin(), updateables.end(),
    [](const std::shared_ptr<IUpdateable>& updateable)
    {
      return std::dynamic_pointer_cast<Volume>(updateable) != nullptr;
    }),
    updateables.end());
}

/**
 * @brief Set the skybox for the scene
 * 
 * @param cubemap The cube map for the skybox
 */
void Scene::SetSkybox(std::shared_ptr<TextureCube> cubemap)
{
  if (cubemap != nullptr && cubemap->IsValid())
  {
    skybox = std::make_shared<Skybox>(std::move(cubemap));
    return;
  }

  skybox.reset();
}

/**
 * @brief Set the skybox for the scene
 * 
 * @param faces The faces of the skybox
 */
void Scene::SetSkybox(const SkyboxFaces& faces)
{
  SetSkybox(std::make_shared<TextureCube>(faces));
}

/**
 * @brief Clear the skybox for the scene
 * 
 */
void Scene::ClearSkybox()
{
  skybox.reset();
}

/**
 * @brief Register a shader with the scene for hot reload tracking
 * 
 * @param name The identifier name for this shader
 * @param shader The shader to track
 */
void Scene::RegisterShader(const std::string& name, std::shared_ptr<Shader> shader)
{
  if (!shader)
  {
    return;
  }

  for (auto& registeredShader : shaders)
  {
    if (registeredShader && registeredShader->GetId() == name)
    {
      registeredShader = std::move(shader);
      return;
    }
  }

  shaders.emplace_back(std::move(shader));
}

/**
 * @brief Hot reload: check all shader files for modifications and reload if changed
 * 
 */
void Scene::ReloadShadersIfChanged()
{
  for (auto& shader : shaders)
  {
    if (shader && shader->ReloadIfChanged())
    {
      // Shader was reloaded successfully
    }
  }
}

/**
 * @brief Set the aspect ratio for the scene's camera
 * 
 * @param aspect The aspect ratio
 */
void Scene::SetCameraAspect(float aspect)
{
  if (camera)
  {
    camera->SetAspect(aspect);
  }
}

/**
 * @brief Destroy the Scene:: Scene object
 * 
 */
void Scene::Destroy()
{
}

/**
 * @brief Destroy the Scene:: Scene object
 * 
 */
Scene::~Scene()
{
}

std::vector<RenderProxy> Scene::GetRenderProxies() const
{
  std::vector<RenderProxy> proxies;

  CompositeUniformProvider sceneUniforms;
  sceneUniforms.AddProvider(*this);
  if (camera)
  {
    sceneUniforms.AddProvider(*camera);
  }

  int lightUniformIndex = 0;
  for (const auto& light : lights)
  {
    if (!light || !light->GetEnabled())
    {
      continue;
    }

    light->SetUniformIndex(lightUniformIndex++);
    sceneUniforms.AddProvider(*light);
  }

  for (const auto &drawable : drawables)
  {
    if (!drawable)
    {
      continue;
    }

    RenderProxy proxy;
    proxy.frameUniforms = sceneUniforms;
    proxy.preferredShader = nullptr;
    proxy.geometry = nullptr;
    proxy.visible = true;

    drawable->BuildRenderProxy(proxy);

    if (proxy.visible && proxy.geometry != nullptr)
    {
      proxies.push_back(proxy);
    }
  }

  return proxies;
}

/**
 * @brief Rebuild the inspect providers for the scene, including the scene itself, 
 *  the camera, and all lights and drawables that implement InspectProvider
 * 
 */
void Scene::RebuildInspectProviders()
{
  inspectProviders.clear();
  AddInspectProvider(this);
  AddInspectProvider(camera);

  for (const auto &light : lights)
  {
    AddInspectProvider(light);
  }

  for (const auto& drawable : drawables)
  {
    if (auto provider = std::dynamic_pointer_cast<InspectProvider>(drawable))
    {
      AddInspectProvider(provider);
    }
  }
}

/**
 * @brief Get the inspect fields for the scene, clear color in this case
 * 
 * @return std::vector<std::shared_ptr<IInspectWidget>> 
 */
std::vector<std::shared_ptr<IInspectWidget>> Scene::GetInspectFields()
{
  std::vector<std::shared_ptr<IInspectWidget>> fields;

  auto clearColorField = std::make_shared<InspectColorFieldWidget>("clearColor", "Clear Color", "Color");
  clearColorField->SetValue(QVariantList{clearColor[0], clearColor[1], clearColor[2]});
  clearColorField->valueChangedCallback = [this](const QVariant &value)
  {
    const QVariantList list = value.toList();
    if (list.size() >= 3)
    {
      clearColor[0] = static_cast<float>(list[0].toDouble());
      clearColor[1] = static_cast<float>(list[1].toDouble());
      clearColor[2] = static_cast<float>(list[2].toDouble());
    }
  };
  fields.push_back(clearColorField);

  return fields;
}
