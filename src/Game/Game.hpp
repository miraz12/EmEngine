#pragma once

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif
#include <GLFW/glfw3.h>

#include "Engine/ECS/ECSManager.hpp"
#include "Engine/Objects/Quad.hpp"

class Game {
public:
  Game(GLFWwindow *window);
  ~Game();

  void update(float dt);
  void reset();

private:
  void init();
  void setupEntities();

  GLFWwindow *m_window;
  ECSManager *m_ECSManager;
};
