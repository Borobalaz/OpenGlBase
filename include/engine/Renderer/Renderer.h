#pragma once
#include <memory>
#include <vector>
#include "Postprocessing/IPostProcessingEffect.h"
#include "Renderer/RenderProxy.h"

class Scene;

class Renderer
{
public:
  Renderer() = default;
  virtual void Render(Scene& scene) = 0;
  void AddPostProcessingEffect(std::shared_ptr<IPostProcessingEffect> effect) { postProcessingEffects.push_back(effect); }
  void ClearPostProcessingEffects( ) { postProcessingEffects.clear(); }
private:
  std::vector<std::shared_ptr<IPostProcessingEffect>> postProcessingEffects;
};