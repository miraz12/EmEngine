#include "InputManager.hpp"

InputManager::InputManager() {
  keys.insert({KEY::Escape, false});
  keys.insert({KEY::W, false});
  keys.insert({KEY::A, false});
  keys.insert({KEY::S, false});
  keys.insert({KEY::D, false});
  keys.insert({KEY::F, false});
  keys.insert({KEY::O, false});
  keys.insert({KEY::Space, false});
  keys.insert({KEY::ArrowUp, false});
  keys.insert({KEY::ArrowDown, false});
  keys.insert({KEY::ArrowRight, false});
  keys.insert({KEY::ArrowLeft, false});
  keys.insert({KEY::Mouse1, false});
}

void InputManager::HandleInput(KEY k, bool pressed) { keys.at(k) = pressed; }
