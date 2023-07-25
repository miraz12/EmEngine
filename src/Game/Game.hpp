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
  // TODO: Dont hardcode this as it defeats the purpose of having dir light set in scene file
  glm::vec3 dirLightColor{0.988f, 0.898f, 0.439f};
  float dirLightAmbient{0.1};
  glm::vec3 dirLightDir{0.01, -1.0, 0.0};
  int32_t debugView{0};
  bool debugMode{0};
  Entity selectedEntity{0};
  ECSManager &m_ECSManager;

private:
  void init();
  void setupEntities();
  void handleInput(float dt);

  GLFWwindow &m_window;
  InputManager &m_InputManager;
  float m_pitch{0.0f};
  float m_yaw{-90.0f};
  int32_t m_mousePosX, m_mousePosY;
  Entity m_player{0};
};

#endif // GAME_H_
