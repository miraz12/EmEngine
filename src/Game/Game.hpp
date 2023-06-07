#ifndef GAME_H_
#define GAME_H_

#include "ECS/ECSManager.hpp"
#include "InputManager.hpp"
#include "glm/fwd.hpp"

class Game {
public:
  Game(GLFWwindow &window);
  ~Game();

  void update(float dt);
  void reset();
  void setPitchYaw(float p, float y) {
    m_pitch = p;
    m_yaw = y;
  }
  void setMousePos(float x, float y) {
    m_mousePosX = x;
    m_mousePosY = y;
  }
  void setViewport(uint32_t w, uint32_t h);
  // Dir light
  std::shared_ptr<DirectionalLight> dLight;
  glm::vec3 dirLightColor;
  float dirLightAmbient;
  glm::vec3 dirLightDir;
  int32_t debugView{0};
  bool debugMode{0};

private:
  void init();
  void setupEntities();
  void handleInput(float dt);

  ECSManager &m_ECSManager;
  GLFWwindow &m_window;
  InputManager &m_InputManager;
  float m_pitch{0.0f};
  float m_yaw{-90.0f};
  int32_t m_mousePosX, m_mousePosY;
};

#endif // GAME_H_
