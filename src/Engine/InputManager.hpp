#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

enum class KEY {
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

  void update(float dt);
  void handleInput(i32 key, i32 action);
  void handleAction(KEY key, i32 action);
  void setMousePos(double x, double y);
  bool getKey(KEY k) { return m_keys.at(k); }
  KEY *getActive() { return m_active.data(); };

  std::vector<KEY> m_active;
  std::unordered_map<KEY, bool> m_keys;

private:
  i32 m_mousePosX;
  i32 m_mousePosY;
  double lastX;
  double lastY;
  double m_pitch{0.0f};
  double m_yaw{-90.0f};
};

#endif // INPUTMANAGER_H_
