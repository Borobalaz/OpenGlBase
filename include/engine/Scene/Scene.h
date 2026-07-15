#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include "Engine.h"
#include "Camera.h"

struct RenderWorld;
class SceneObject;

class Scene
{
public:
  void update(FrameContext& context);
  RenderWorld buildRenderWorld() const;

private:
  std::unique_ptr<Camera> camera;
  std::vector<SceneObject> sceneObjects;
};

