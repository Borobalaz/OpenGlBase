#pragma once
#include <memory>
#include <vector>
#include "Postprocessing/IPostProcessingEffect.h"
#include "RenderCore/RenderFrame.h"
#include "RenderCore/RendererDescriptor.h"

class Renderer
{
public:
  Renderer() = default;
  virtual const RendererDescriptor& GetDescriptor() const = 0;
  virtual void Draw(const RenderFrame& frame) = 0;
  void AddPostProcessingEffect(std::shared_ptr<IPostProcessingEffect> effect) { postProcessingEffects.push_back(effect); }
  void ClearPostProcessingEffects( ) { postProcessingEffects.clear(); }
private:
  std::vector<std::shared_ptr<IPostProcessingEffect>> postProcessingEffects;
};