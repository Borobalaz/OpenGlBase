#pragma once
#include <unordered_map>
#include <vector>
#include <map>

#include "Geometry/Triangle.h"
#include "Geometry/QuadGeometry.h"
#include "Shader.h"
#include "Camera/Camera.h"
#include "Camera/PerspectiveCamera.h"
#include "Uniform/CompositeUniformProvider.h"
#include "Light/PointLight.h"
#include "Light/DirectionalLight.h"
#include "Light/Light.h"
#include "GameObject.h"
#include "Material.h"
#include "Texture/TextureCube.h"
#include "Uniform/UniformProvider.h"
#include "Volume/Volume.h"
#include <memory>

class Shader;
class Triangle;
class Skybox;
class TextureCube;
class Volume;

class Scene : public UniformProvider
{
public:
  Scene();
  ~Scene();
  
  void Init();
  void Update(float deltaTime);
  void Render();
  void Destroy();
  void Apply(Shader& shader) const override;

  // Skybox management
  void SetSkybox(std::shared_ptr<TextureCube> cubemap);
  void SetSkybox(const SkyboxFaces& faces);
  void ClearSkybox();

  // Camera management
  std::shared_ptr<Camera> GetCamera();
  void SetCameraAspect(float aspect);

  // Scene content management
  void AddLight(std::shared_ptr<Light> light);
  void ClearLights();

  void AddGameObject(std::shared_ptr<GameObject> gameObject);
  void ClearGameObjects();

  void AddVolume(std::shared_ptr<Volume> volume);
  void ClearVolumes();

  // Shader management
  void RegisterShader(const std::string& name, std::shared_ptr<Shader> shader);

  // Hot reload: recheck all shader files and reload if changed
  void ReloadShadersIfChanged();

private:
  float clearColor[4];

  std::shared_ptr<Camera> camera;
  CompositeUniformProvider frameUniforms;

  std::shared_ptr<Skybox> skybox;

  std::vector<std::shared_ptr<Light>> lights;
  std::vector<std::shared_ptr<Volume>> volumes;
  std::vector<std::shared_ptr<GameObject>> gameObjects;
  std::map<std::string, std::shared_ptr<Shader>> shaders;

  glm::vec3 matrixTestUniformValue;
};
