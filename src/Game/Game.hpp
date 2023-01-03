#pragma once

#include "Engine/ECS/ECSManager.hpp"
#include "Engine/InputManager.hpp"

class Game {
public:
  Game(GLFWwindow *window);
  ~Game();

  void update(float dt);
  void reset();
  void setPitchYaw(float p, float y) {
    m_pitch = p;
    m_yaw = y;
  }

private:
  void init();
  void setupEntities();
  void handleInput(float dt);

  GLFWwindow *m_window;
  ECSManager *m_ECSManager;
  InputManager *m_InputManager;
  float m_pitch{0.0f};
  float m_yaw{-90.0f};
};
