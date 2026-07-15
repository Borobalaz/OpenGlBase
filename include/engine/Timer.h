#pragma once

class Timer
{
public:
  // Playback control
  void play();
  void pause();

  // Step / direction control
  void setForward();
  void stepBackward();

  // Time control
  void setTimeScale(float scale);

  // Query
  float deltaTime() const;

private:
  float m_deltaTime = 0.0f;
  bool m_paused = false;
  float m_timeScale = 1.0f;
  bool m_stepRequested = false;
  int m_stepDirection = 1; // 1 = forward, -1 = backward
  float m_stepSize = 1.0f; // seconds per step
};
