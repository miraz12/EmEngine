#ifndef INPUTMANAGER_H_
#define INPUTMANAGER_H_

#include <unordered_map>

class InputManager {
public:
  static InputManager &getInstance() {
    static InputManager instance;
    return instance;
  }
  InputManager(InputManager const &) = delete;
  void operator=(InputManager const &) = delete;


  enum KEY {
    Escape,
    W,
    A,
    S,
    D,
    F,
    ArrowUp,
    ArrowDown,
    ArrowRight,
    ArrowLeft,
    Mouse1 
  };

  void HandleInput(KEY k, bool pressed);

  std::unordered_map<KEY, bool> keys;

 private:
  InputManager();
};

#endif // INPUTMANAGER_H_