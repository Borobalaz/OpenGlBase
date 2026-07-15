#pragma once
#include <vector>
#include <memory>
#include "Timer.h"
#include "InputHandler.h"

class Renderer;

struct FrameContext
{
  Timer* timer;
  InputHandler* inputState;
};

class Engine
{
public: 
  void update();
  void render();

private:
  std::vector<std::unique_ptr<class Scene>> scenes;
  Scene* activeScene = nullptr;
  std::unique_ptr<Renderer> renderer;
  Timer timer;
  InputHandler inputHandler;
};