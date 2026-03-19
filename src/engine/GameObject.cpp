#include "GameObject.h"

#include <glm/gtc/matrix_transform.hpp>

#include "CompositeUniformProvider.h"

GameObject::GameObject()
{
  position = glm::vec3(0.0f);
  rotation = glm::vec3(0.0f);
  scale = glm::vec3(1.0f);
}

GameObject::~GameObject()
{
}

void GameObject::AddMesh(std::shared_ptr<Mesh> mesh)
{
  if (!mesh)
  {
    return;
  }

  meshes.push_back(std::move(mesh));
}

void GameObject::Update(float deltaTime)
{
}

void GameObject::Draw(const UniformProvider& frameUniforms) const
{
  if (meshes.empty() || !visible)
  {
    return;
  }

  CompositeUniformProvider compositeProvider;
  compositeProvider.AddProvider(frameUniforms);
  compositeProvider.AddProvider(*this);

  for (const auto& mesh : meshes)
  {
    if (mesh)
    {
      mesh->Draw(compositeProvider);
    }
  }
}

void GameObject::Apply(Shader& shader) const
{
  const glm::mat4 modelMatrix = BuildModelMatrix();
  shader.SetMat4(ComposeUniformName("gameObject", "modelMatrix"), modelMatrix);
}

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

void GameObject::CollectInspectableFields(std::vector<UiField>& out, const std::string& groupPrefix)
{
  const std::string group = groupPrefix.empty() ? "GameObject" : groupPrefix;

  UiField positionField;
  positionField.group = group;
  positionField.label = "Position";
  positionField.kind = UiFieldKind::Vec3;
  positionField.speed = 0.01f;
  positionField.getter = [this]() -> UiFieldValue
  {
    return position;
  };
  positionField.setter = [this](const UiFieldValue& value)
  {
    if (!std::holds_alternative<glm::vec3>(value))
    {
      return;
    }

    position = std::get<glm::vec3>(value);
  };
  out.push_back(std::move(positionField));

  UiField rotationField;
  rotationField.group = group;
  rotationField.label = "Rotation";
  rotationField.kind = UiFieldKind::Vec3;
  rotationField.speed = 0.01f;
  rotationField.getter = [this]() -> UiFieldValue
  {
    return rotation;
  };
  rotationField.setter = [this](const UiFieldValue& value)
  {
    if (!std::holds_alternative<glm::vec3>(value))
    {
      return;
    }

    rotation = std::get<glm::vec3>(value);
  };
  out.push_back(std::move(rotationField));

  UiField scaleField;
  scaleField.group = group;
  scaleField.label = "Scale";
  scaleField.kind = UiFieldKind::Vec3;
  scaleField.speed = 0.01f;
  scaleField.getter = [this]() -> UiFieldValue
  {
    return scale;
  };
  scaleField.setter = [this](const UiFieldValue& value)
  {
    if (!std::holds_alternative<glm::vec3>(value))
    {
      return;
    }

    scale = std::get<glm::vec3>(value);
  };
  out.push_back(std::move(scaleField));

  UiField visibleField;
  visibleField.group = group;
  visibleField.label = "Visible";
  visibleField.kind = UiFieldKind::Bool;
  visibleField.getter = [this]() -> UiFieldValue
  {
    return visible;
  };
  visibleField.setter = [this](const UiFieldValue& value)
  {
    if (!std::holds_alternative<bool>(value))
    {
      return;
    }

    visible = std::get<bool>(value);
  };
  out.push_back(std::move(visibleField));

}