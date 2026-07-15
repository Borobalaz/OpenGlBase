#pragma once

#include <unordered_map>
#include <glm/vec2.hpp>

using Key = int;

class InputHandler
{
public:
  // Query key/mouse state
  bool IsKeyDown(Key key) const;
  bool WasKeyPressed(Key key) const;
  const glm::vec2& MousePosition() const;

  // Modifiers (implementation in .cpp)
  void SetKeyState(Key key, bool down);
  void SetMousePosition(const glm::vec2& pos);
  void AddScrollDelta(float delta);
  void AdvanceFrame();

private:
  std::unordered_map<Key, bool> pressedKeys;
  std::unordered_map<Key, bool> previousPressedKeys;
  glm::vec2 mousePos{0.0f, 0.0f};
  glm::vec2 previousMousePos{0.0f, 0.0f};
  float scrollDelta = 0.0f;
};
