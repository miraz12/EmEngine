#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

enum class KEY
{
  Escape,
  W,
  A,
  S,
  D,
  Space,
  F,
  O,
  T,
  ArrowUp,
  ArrowDown,
  ArrowRight,
  ArrowLeft,
  Mouse1,
  Key1,
  Key2,
  Key3,
  Key4,
  LeftShift,
  Q,
  E
};

class InputManager : public Singleton<InputManager>
{
  friend class Singleton<InputManager>;

public:
  InputManager();
  ~InputManager() = default;

  void update(float dt);
  void handleInput(i32 key, i32 action);
  void handleAction(KEY key, i32 action);
  void setMousePos(double x, double y);
  bool getKey(KEY k) { return m_keys.at(k); }
  KEY* getActive() { return m_active.data(); };
  double getMouseX() const { return m_mousePosX; }
  double getMouseY() const { return m_mousePosY; }
  double getLastMouseX() const { return lastX; }
  double getLastMouseY() const { return lastY; }
  double getMouseDeltaX() const { return m_mouseDeltaX; }
  double getMouseDeltaY() const { return m_mouseDeltaY; }

  std::vector<KEY> m_active;
  std::unordered_map<KEY, bool> m_keys;

private:
  i32 m_mousePosX;
  i32 m_mousePosY;
  double lastX;
  double lastY;
  double m_mouseDeltaX{ 0.0 };
  double m_mouseDeltaY{ 0.0 };
  double m_pitch{ 0.0f };
  double m_yaw{ -90.0f };
};

#endif // INPUTMANAGER_H_
