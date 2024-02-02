#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

enum KEY {
  Escape,
  W,
  A,
  S,
  D,
  F,
  O,
  Space,
  ArrowUp,
  ArrowDown,
  ArrowRight,
  ArrowLeft,
  Mouse1
};

class InputManager : public Singleton<InputManager> {
  friend class Singleton<InputManager>;

public:
  InputManager();
  ~InputManager() = default;

  void handleInput(i32 key, i32 action);
  void setMousePos(double x, double y);
  bool getKey(KEY k) { return m_keys.at(k); }

private:
  i32 m_mousePosX, m_mousePosY;
  std::unordered_map<KEY, bool> m_keys;
};

#endif // INPUTMANAGER_H_
