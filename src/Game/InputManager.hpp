#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

#include <Singleton.hpp>
#include <unordered_map>

class InputManager : public Singleton<InputManager> {
  friend class Singleton<InputManager>;

public:
  enum KEY { Escape, W, A, S, D, F, O, ArrowUp, ArrowDown, ArrowRight, ArrowLeft, Mouse1 };

  void HandleInput(KEY k, bool pressed);

  std::unordered_map<KEY, bool> keys;

private:
  InputManager();
  ~InputManager() = default;
};

#endif // INPUTMANAGER_H_
