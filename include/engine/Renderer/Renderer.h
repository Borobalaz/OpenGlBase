#pragma once
#include <memory>
#include <vector>
#include "postprocessing\IPostProcessingEffect.h"

struct RenderWorld;

class Renderer
{
public:
  Renderer() = default;
  virtual ~Renderer() = default;

  // Lifecycle
  virtual void init() {}
  virtual void resize(int width, int height) {}

  // Render a fully-built RenderWorld produced by a Scene
  virtual void render(const RenderWorld& world) = 0;

  void addPostProcessingEffect(std::shared_ptr<IPostProcessingEffect> effect) { postProcessingEffects.push_back(effect); }
  void clearPostProcessingEffects() { postProcessingEffects.clear(); }

private:
  std::vector<std::shared_ptr<IPostProcessingEffect>> postProcessingEffects;
};