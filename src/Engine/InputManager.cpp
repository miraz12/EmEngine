#include "InputManager.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <algorithm>
#include <ranges>

InputManager::InputManager()
{
  m_keys.insert({ KEY::Escape, false });
  m_keys.insert({ KEY::W, false });
  m_keys.insert({ KEY::A, false });
  m_keys.insert({ KEY::S, false });
  m_keys.insert({ KEY::D, false });
  m_keys.insert({ KEY::F, false });
  m_keys.insert({ KEY::O, false });
  m_keys.insert({ KEY::T, false });
  m_keys.insert({ KEY::Space, false });
  m_keys.insert({ KEY::ArrowUp, false });
  m_keys.insert({ KEY::ArrowDown, false });
  m_keys.insert({ KEY::ArrowRight, false });
  m_keys.insert({ KEY::ArrowLeft, false });
  m_keys.insert({ KEY::Mouse1, false });
  m_keys.insert({ KEY::Key1, false });
  m_keys.insert({ KEY::Key2, false });
  m_keys.insert({ KEY::Key3, false });
  m_keys.insert({ KEY::Key4, false });
  m_keys.insert({ KEY::LeftShift, false });
  m_keys.insert({ KEY::Q, false });
  m_keys.insert({ KEY::E, false });
}

void
InputManager::update(float dt)
{
  // float camSpeed = 1.f;
  // float moveSpeed = 0.25f;
  ECSManager& ecsManager = ECSManager::getInstance();
  if (m_keys.at(KEY::O)) {
    ecsManager.setSimulatePhysics(ecsManager.getSimulatePhysics() ? false
                                                                  : true);
  }

  // Handle object picking when mouse is clicked
  static bool pressed = true;
  if (m_keys.at(KEY::Mouse1)) {
    if (pressed) {
      PhysicsSystem::getInstance().performPicking(m_mousePosX, m_mousePosY);
      pressed = false;
    }
  } else {
    pressed = true;
  }
}

void
InputManager::handleInput(i32 key, i32 action)
{
  switch (key) {
    case GLFW_KEY_W:
      handleAction(KEY::W, action);
      break;
    case GLFW_KEY_A:
      handleAction(KEY::A, action);
      break;
    case GLFW_KEY_S:
      handleAction(KEY::S, action);
      break;
    case GLFW_KEY_D:
      handleAction(KEY::D, action);
      break;
    case GLFW_KEY_F:
      handleAction(KEY::F, action);
      break;
    case GLFW_KEY_O:
      handleAction(KEY::O, action);
      break;
    case GLFW_KEY_T:
      handleAction(KEY::T, action);
      break;
    case GLFW_KEY_SPACE:
      handleAction(KEY::Space, action);
      break;
    case GLFW_KEY_UP:
      handleAction(KEY::ArrowUp, action);
      break;
    case GLFW_KEY_DOWN:
      handleAction(KEY::ArrowDown, action);
      break;
    case GLFW_KEY_RIGHT:
      handleAction(KEY::ArrowRight, action);
      break;
    case GLFW_KEY_LEFT:
      handleAction(KEY::ArrowLeft, action);
      break;
    case GLFW_MOUSE_BUTTON_1:
      handleAction(KEY::Mouse1, action);
      break;
    case GLFW_KEY_1:
      handleAction(KEY::Key1, action);
      break;
    case GLFW_KEY_2:
      handleAction(KEY::Key2, action);
      break;
    case GLFW_KEY_3:
      handleAction(KEY::Key3, action);
      break;
    case GLFW_KEY_4:
      handleAction(KEY::Key4, action);
      break;
    case GLFW_KEY_LEFT_SHIFT:
      handleAction(KEY::LeftShift, action);
      break;
    case GLFW_KEY_Q:
      handleAction(KEY::Q, action);
      break;
    case GLFW_KEY_E:
      handleAction(KEY::E, action);
      break;
    default:
      break;
  }
}

void
InputManager::handleAction(KEY key, i32 action)
{
  m_keys.at(key) = action;
}

void
InputManager::setMousePos(double x, double y)
{
  m_mousePosX = x;
  m_mousePosY = y;

  if (!m_keys.at(KEY::Mouse1)) {
    lastX = x;
    lastY = y;
    m_mouseDeltaX = 0.0;
    m_mouseDeltaY = 0.0;
    return;
  }

  // Calculate delta for this frame
  m_mouseDeltaX = x - lastX;
  m_mouseDeltaY = lastY - y; // Inverted Y
  lastX = x;
  lastY = y;

  // Old camera control code (kept for reference, not used)
  float sensitivity = 0.1f;
  double xoffset = m_mouseDeltaX * sensitivity;
  double yoffset = m_mouseDeltaY * sensitivity;

  m_yaw += xoffset;
  m_pitch += yoffset;

  m_pitch = std::min<double>(m_pitch, 89.0);
  m_pitch = std::max<double>(m_pitch, -89.0);
}

extern "C" int
GetPressed(int** vec)
{
  InputManager::getInstance().m_active.clear();
  for (KEY key :
       InputManager::getInstance().m_keys |
         std::views::filter([](const auto& entry) { return entry.second; }) |
         std::views::transform([](const auto& entry) { return entry.first; })) {
    InputManager::getInstance().m_active.push_back(key);
  }

  *vec = reinterpret_cast<int*>(InputManager::getInstance().m_active.data());
  return InputManager::getInstance().m_active.size();
}

extern "C" void
GetMousePosition(float* x, float* y)
{
  InputManager& inputMgr = InputManager::getInstance();
  *x = static_cast<float>(inputMgr.getMouseX());
  *y = static_cast<float>(inputMgr.getMouseY());
}

extern "C" void
GetMouseDelta(float* deltaX, float* deltaY)
{
  InputManager& inputMgr = InputManager::getInstance();

  // Return the deltas calculated in setMousePos
  *deltaX = static_cast<float>(inputMgr.getMouseDeltaX());
  *deltaY = static_cast<float>(inputMgr.getMouseDeltaY());
}
