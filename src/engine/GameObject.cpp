#include "GameObject.h"

#include <algorithm>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

#include <cmath>

#include "Uniform/CompositeUniformProvider.h"
#include "ui/widgets/inspect_fields/InspectCheckboxFieldWidget.h"
#include "ui/widgets/inspect_fields/InspectVec3FieldWidget.h"

GameObject::GameObject(const std::string& id)
  : id(id)
{
}

GameObject::GameObject(std::shared_ptr<Geometry> geometry, std::shared_ptr<Material> material, const std::string& id)
  : id(id)
{
  if (geometry && material)
  {
    auto mesh = std::make_shared<Mesh>(geometry, material);
    AddMesh(mesh);
  }
}

GameObject::~GameObject()
{
}

/**
 * @brief Push back a mesh on the game object.
 *
 * @param mesh
 */
void GameObject::AddMesh(std::shared_ptr<Mesh> mesh)
{
  if (!mesh)
  {
    return;
  }

  meshes.push_back(std::move(mesh));
}

void GameObject::SetMaterial(std::shared_ptr<Material> material)
{
  if (!material)
  {
    return;
  }

  for (const auto& mesh : meshes)
  {
    if (mesh)
    {
      mesh->SetMaterial(material);
    }
  }
}

/**
 * @brief Render the game object's meshes with the provided uniforms.
 *
 * @param frameUniforms
 */
void GameObject::Draw(const UniformProvider &frameUniforms) const
{
  if (meshes.empty() || !visible)
  {
    return;
  }

  CompositeUniformProvider compositeProvider;
  compositeProvider.AddProvider(frameUniforms);
  compositeProvider.AddProvider(*this);

  for (const auto &mesh : meshes)
  {
    if (mesh)
    {
      mesh->Draw(compositeProvider);
    }
  }
}

/**
 * @brief UniformProvider implementation. Set the gameObject uniforms in the shader.
 *
 * @param shader
 */
void GameObject::Apply(Shader &shader) const
{
  const glm::mat4 modelMatrix = BuildModelMatrix();
  shader.SetMat4(ComposeUniformName("gameObject", "modelMatrix"), modelMatrix);
}

/**
 * @brief Construct the model matrix from the transform of the gameObject.
 *
 * @return glm::mat4
 */
glm::mat4 GameObject::BuildModelMatrix() const
{
  return transform.GetModelMatrix();
}

/**
 * @brief Get the string which identifies the object. 
 * 
 * @return std::string 
 */
std::string GameObject::GetInspectDisplayName() const
{
  return id.empty() ? std::string("Game Object") : id;
}

/**
 * @brief Get the fields that should be visible in the inspector: position, rotation, scale, and visibility.
 * 
 * @return std::vector<std::shared_ptr<IInspectWidget>> 
 */
std::vector<std::shared_ptr<IInspectWidget>> GameObject::GetInspectFields()
{
  std::vector<std::shared_ptr<IInspectWidget>> fields = transform.GetInspectFields();

  auto visibleField = std::make_shared<InspectCheckboxFieldWidget>("isVisible", "Visible", "Rendering");
  visibleField->SetValue(visible);
  visibleField->valueChangedCallback = [this](const QVariant &value)
  {
    visible = value.toBool();
  };

  fields.push_back(visibleField);
  return fields;
}

/**
 * @brief Cast a ray against the gameObject and return the distance to the intersection point.
 *        Use a bounding sphere for intersection test, with the radius being the maximum scale component.
 * 
 * @param rayOrigin 
 * @param rayDirection 
 * @return std::optional<float> 
 */
std::optional<float> GameObject::CastRay(const glm::vec3 &rayOrigin, const glm::vec3 &rayDirection) const
{
  if (!visible)
  {
    return std::nullopt;
  }

  const float radius = std::max({std::abs(transform.GetScale().x), std::abs(transform.GetScale().y), std::abs(transform.GetScale().z), 0.1f});
  const glm::vec3 toCenter = rayOrigin - transform.GetPosition();

  const float a = glm::dot(rayDirection, rayDirection);
  const float b = 2.0f * glm::dot(toCenter, rayDirection);
  const float c = glm::dot(toCenter, toCenter) - radius * radius;
  const float discriminant = b * b - 4.0f * a * c;

  if (discriminant < 0.0f)
  {
    return std::nullopt;
  }

  const float sqrtDiscriminant = std::sqrt(discriminant);
  const float t0 = (-b - sqrtDiscriminant) / (2.0f * a);
  const float t1 = (-b + sqrtDiscriminant) / (2.0f * a);

  if (t0 >= 0.0f)
  {
    return t0;
  }

  if (t1 >= 0.0f)
  {
    return t1;
  }

  return std::nullopt;
}


