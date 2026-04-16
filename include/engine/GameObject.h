#pragma once

#include <memory>
#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "Mesh.h"
#include "IDrawable.h"
#include "Uniform/UniformProvider.h"
#include "ui/mediator/InspectProvider.h"
#include "ui/mediator/InspectField.h"

class GameObject : public UniformProvider, public IDrawable, public InspectProvider
{
public:
  explicit GameObject(const std::string& id);
  ~GameObject();

  void AddMesh(std::shared_ptr<Mesh> mesh);

  void Update(float deltaTime);
  void Draw(const UniformProvider& frameUniforms) const override;
  void Apply(Shader& shader) const override;

  void SetPosition(const glm::vec3& pos) { position = pos; }
  void SetRotation(const glm::vec3& rot) { rotation = rot; }
  void SetScale(const glm::vec3& s) { scale = s; }
  const glm::vec3& GetPosition() const { return position; }
  const glm::vec3& GetRotation() const { return rotation; }
  const glm::vec3& GetScale() const { return scale; }
  const std::string& GetId() const { return id; }

  std::string GetInspectDisplayName() const override;
  std::vector<std::shared_ptr<InspectField>> GetInspectFields() override;

private:
  glm::vec3 position;
  glm::vec3 rotation;
  glm::vec3 scale;

  glm::mat4 BuildModelMatrix() const;

  const std::string id;
  std::vector<std::shared_ptr<Mesh>> meshes;
};