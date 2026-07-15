#include "Engine.h"

void Engine::render()
{
  if (activeScene && renderer)
  {
    renderer->render(*activeScene);
  }
}

void Engine::update()
{
  if (activeScene)
  {
    FrameContext frameContext;
    frameContext.timer = &timer;
    frameContext.inputState = &inputHandler;
    activeScene->update(frameContext);
  }
}