#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

#include <ECS/ECSManager.hpp>
#include <Singleton.hpp>
#include <cstdint>
#include <unordered_map>

class InputManager : public Singleton<InputManager> {
  friend class Singleton<InputManager>;

public:
  enum KEY { Escape, W, A, S, D, F, O, Space, ArrowUp, ArrowDown, ArrowRight, ArrowLeft, Mouse1 };

  void handleInput(KEY k, bool pressed);
  void update(float dt);
  void setPitchYaw(float p, float y) {
    m_pitch = p;
    m_yaw = y;
  }
  void setMousePos(float x, float y) {
    m_mousePosX = x;
    m_mousePosY = y;
  }

  std::unordered_map<KEY, bool> keys;

private:
  InputManager();
  ~InputManager() = default;
  float m_pitch{0.0f};
  float m_yaw{-90.0f};
  i32 m_mousePosX, m_mousePosY;
  Entity m_player{0};
};

#endif // INPUTMANAGER_H_
