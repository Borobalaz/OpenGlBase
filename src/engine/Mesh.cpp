#include "Mesh.h"

#include "Uniform/CompositeUniformProvider.h"

Mesh::Mesh(std::shared_ptr<Geometry> geometry,
           std::shared_ptr<Material> material)
  : geometry(std::move(geometry)),
    material(std::move(material))
{
}

/**
 * @brief Set the argument pointer's destination as the mesh's geometry field pointer destination. 
 *        The argument pointer can be recycled.
 * 
 * @param geometry 
 */
void Mesh::SetGeometry(std::shared_ptr<Geometry> geometry)
{
  this->geometry = std::move(geometry);
}

/**
 * @brief Set the argument pointer's destination as the mesh's material field pointer destination. 
 *        The argument pointer can be recycled.
 * 
 * @param material 
 */
void Mesh::SetMaterial(std::shared_ptr<Material> material)
{
  this->material = std::move(material);
}

/**
 * @brief Render the mesh.
 * 
 * Composes frame-level uniforms (camera, lights) with material properties
 * into a single composite provider to bind to the shader.
 * 
 * @param uniformProvider Frame-level uniform provider (lights, camera, etc.)
 */
void Mesh::Draw(const UniformProvider& uniformProvider) const
{
  if (!geometry || !material)
  {
    return;
  }

  Shader& shader = material->GetShader();
  shader.Use();

  // Compose frame uniforms with material properties
  CompositeUniformProvider composite;
  composite.AddProvider(uniformProvider);  // Lights, camera, scene
  composite.AddProvider(*material);         // Material PBR properties and textures

  composite.Apply(shader);
  geometry->Draw(shader);
}