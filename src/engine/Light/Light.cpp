#include "Light/Light.h"

#include "ui/mediator/InspectBoolField.h"
#include "ui/mediator/InspectColorField.h"

Light::Light(const std::string& id,
             const glm::vec3& ambient,
             const glm::vec3& diffuse,
             const glm::vec3& specular)
  : ambient(ambient),
    diffuse(diffuse),
    specular(specular),
    uniformIndex(-1),
    id(id)
{
}

std::string Light::GetInspectDisplayName() const
{
  return id;
}

std::vector<std::shared_ptr<InspectField>> Light::GetInspectFields()
{
  return {
    std::make_shared<InspectBoolField>("enabled",
                                       "Enabled",
                                       "Light",
                                       [this]() { return enabled; },
                                       [this](bool value) { enabled = value; }),
    std::make_shared<InspectColorField>("ambient",
                                        "Ambient",
                                        "Color",
                                        [this]() { return ambient; },
                                        [this](const glm::vec3& value) { ambient = value; }),
    std::make_shared<InspectColorField>("diffuse",
                                        "Diffuse",
                                        "Color",
                                        [this]() { return diffuse; },
                                        [this](const glm::vec3& value) { diffuse = value; }),
    std::make_shared<InspectColorField>("specular",
                                        "Specular",
                                        "Color",
                                        [this]() { return specular; },
                                        [this](const glm::vec3& value) { specular = value; })
  };
}

void Light::SetUniformIndex(int index)
{
  uniformIndex = index;
}

int Light::GetUniformIndex() const
{
  return uniformIndex;
}

void Light::Apply(Shader& shader) const
{
  // Base Light class doesn't apply any uniforms
  // Derived classes (PointLight, DirectionalLight) override this method
}

