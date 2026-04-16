#include "GameObject.h"

#include <string>

#include <glm/gtc/matrix_transform.hpp>

#include "Uniform/CompositeUniformProvider.h"
#include "ui/mediator/InspectBoolField.h"
#include "ui/mediator/InspectVec3Field.h"

GameObject::GameObject(const std::string& id)
  : id(id),
    position(glm::vec3(0.0f)),
    rotation(glm::vec3(0.0f)),
    scale(glm::vec3(1.0f))
{
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

/**
 * @brief You know.
 *
 * @param deltaTime
 */
void GameObject::Update(float deltaTime)
{
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
 * @brief Construct the model matrix from the position, rotation and scale of the gameObject.
 *
 * @return glm::mat4
 */
glm::mat4 GameObject::BuildModelMatrix() const
{
  glm::mat4 model = glm::mat4(1.0f);
  model = glm::translate(model, position);
  model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
  model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
  model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
  model = glm::scale(model, scale);
  return model;
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
 * @return std::vector<std::shared_ptr<InspectField>> 
 */
std::vector<std::shared_ptr<InspectField>> GameObject::GetInspectFields()
{
  return {
    std::make_shared<InspectVec3Field>("position",
                                       "Position",
                                       "Transform",
                                       [this]() { return position; },
                                       [this](const glm::vec3& value) { position = value; }),
    std::make_shared<InspectVec3Field>("rotation",
                                       "Rotation",
                                       "Transform",
                                       [this]() { return rotation; },
                                       [this](const glm::vec3& value) { rotation = value; }),
    std::make_shared<InspectVec3Field>("scale",
                                       "Scale",
                                       "Transform",
                                       [this]() { return scale; },
                                       [this](const glm::vec3& value) { scale = value; }),
    std::make_shared<InspectBoolField>("isVisible",
                                       "Visible",
                                       "Rendering",
                                       [this]() { return visible; },
                                       [this](bool value) { visible = value; })
  };
}

