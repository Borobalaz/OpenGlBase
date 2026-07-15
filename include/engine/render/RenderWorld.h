#pragma once

#include <vector>
#include "CameraRenderObject.h"
#include "MeshRenderObject.h"
#include "LightRenderObject.h"
#include "EnvironmentRenderObject.h"

struct RenderWorld
{
  CameraRenderObject camera;
  std::vector<MeshRenderObject> meshes;
  std::vector<LightRenderObject> lights;
  EnvironmentRenderObject environment;
};
