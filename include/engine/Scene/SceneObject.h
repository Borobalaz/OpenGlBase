#pragma once

#include <string>

struct FrameContext;
struct RenderWorld;

class SceneObject
{
public:
  void update(FrameContext& context);
  void extractRenderWorldData(RenderWorld& renderWorld) const;
  // New API for the refactored render pipeline: populate the provided RenderWorld
  // with this object's render representation (camera, meshes, lights, etc.).
  void extractWorldRenderObject(RenderWorld& world) const;

private:
  int uid;
  std::string name;
};