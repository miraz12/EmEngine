#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_
#include <engine_api.hpp>

class InputManager : public Singleton<InputManager>, public API::InputManager {
  friend class Singleton<InputManager>;

public:
  InputManager();
  ~InputManager() = default;

  void handleInput(i32 key, i32 action);
  void setMousePos(double x, double y);
  bool getKey(KEY k) override { return m_keys.at(k); }

private:
  i32 m_mousePosX, m_mousePosY;
  std::unordered_map<KEY, bool> m_keys;
};

#endif // INPUTMANAGER_H_
