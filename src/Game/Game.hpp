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
  void setPitchYaw(float p, float y) { m_InputManager.setPitchYaw(p, y); }
  void setMousePos(float x, float y) { m_InputManager.setMousePos(x, y); }
  void setViewport(u32 w, u32 h);
  // Dir light
  std::shared_ptr<DirectionalLight> dLight;
  // TODO: Dont hardcode this as it defeats the purpose of having dir light set in scene file
  glm::vec3 dirLightColor{0.988f, 0.898f, 0.439f};
  float dirLightAmbient{0.1};
  glm::vec3 dirLightDir{0.01, -1.0, 0.0};
  Entity selectedEntity{0};
  ECSManager &m_ECSManager;

private:
  void init();
  void setupEntities();
  void handleInput(float dt);

  GLFWwindow &m_window;
  InputManager &m_InputManager;
};

#endif // GAME_H_
