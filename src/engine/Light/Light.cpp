#include "Light/Light.h"

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

