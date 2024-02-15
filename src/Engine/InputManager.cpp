#include "InputManager.hpp"

InputManager::InputManager() {
  m_keys.insert({KEY::Escape, false});
  m_keys.insert({KEY::W, false});
  m_keys.insert({KEY::A, false});
  m_keys.insert({KEY::S, false});
  m_keys.insert({KEY::D, false});
  m_keys.insert({KEY::F, false});
  m_keys.insert({KEY::O, false});
  m_keys.insert({KEY::Space, false});
  m_keys.insert({KEY::ArrowUp, false});
  m_keys.insert({KEY::ArrowDown, false});
  m_keys.insert({KEY::ArrowRight, false});
  m_keys.insert({KEY::ArrowLeft, false});
  m_keys.insert({KEY::Mouse1, false});
}

void InputManager::handleInput(i32 key, i32 action) {
  switch (key) {
  case GLFW_KEY_W:
    m_keys.at(KEY::W) = action;
    handleAction(KEY::W, action);
    break;
  case GLFW_KEY_A:
    m_keys.at(KEY::A) = action;
    handleAction(KEY::A, action);
    break;
  case GLFW_KEY_S:
    m_keys.at(KEY::S) = action;
    handleAction(KEY::S, action);
    break;
  case GLFW_KEY_D:
    m_keys.at(KEY::D) = action;
    handleAction(KEY::D, action);
    break;
  case GLFW_KEY_F:
    m_keys.at(KEY::F) = action;
    handleAction(KEY::F, action);
    break;
  case GLFW_KEY_O:
    m_keys.at(KEY::O) = action;
    handleAction(KEY::O, action);
    break;
  case GLFW_KEY_SPACE:
    m_keys.at(KEY::Space) = action;
    handleAction(KEY::Space, action);
    break;
  case GLFW_KEY_UP:
    m_keys.at(KEY::ArrowUp) = action;
    handleAction(KEY::ArrowUp, action);
    break;
  case GLFW_KEY_DOWN:
    m_keys.at(KEY::ArrowDown) = action;
    handleAction(KEY::ArrowDown, action);
    break;
  case GLFW_KEY_RIGHT:
    m_keys.at(KEY::ArrowRight) = action;
    handleAction(KEY::ArrowRight, action);
    break;
  case GLFW_KEY_LEFT:
    m_keys.at(KEY::ArrowLeft) = action;
    handleAction(KEY::ArrowLeft, action);
    break;
  case GLFW_MOUSE_BUTTON_1:
    m_keys.at(KEY::Mouse1) = action;
    handleAction(KEY::Mouse1, action);
    break;
  default:
    break;
  }
}

void InputManager::handleAction(KEY key, i32 action) {
  if (action) {
    m_active.push_back(key);
  }
}

void InputManager::setMousePos(double x, double y) {
  m_mousePosX = x;
  m_mousePosY = y;
}

extern "C" int Input_getPressed(int **vec) {
  *vec = reinterpret_cast<int *>(InputManager::getInstance().getActive());
  return InputManager::getInstance().m_active.size();
}
