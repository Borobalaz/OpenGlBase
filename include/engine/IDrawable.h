#pragma once

#include "Renderer/RenderProxy.h"

class IDrawable
{
public:
  virtual ~IDrawable() = default;

  virtual void BuildRenderProxy(RenderProxy& renderProxy) const = 0;
  bool visible = true;
  void SetVisible(bool isVisible)
  {
    visible = isVisible;
  }

};