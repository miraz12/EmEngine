#include "InputManager.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
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
}

void
InputManager::update(float dt)
{
  // float camSpeed = 1.f;
  // float moveSpeed = 0.25f;
  ECSManager& ecsManager = ECSManager::getInstance();
  // Camera &cam = ecsManager.getCamera();
  // Parse input
  // if (m_keys.at(KEY::A)) {
  //   glm::vec3 camPos =
  //       cam.getPosition() -
  //       glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed *
  //       dt;
  //   if (!glm::all(glm::isnan(camPos))) {
  //     cam.setPosition(camPos);
  //   }
  // }
  // if (m_keys.at(KEY::D)) {
  //   glm::vec3 camPos =
  //       cam.getPosition() +
  //       glm::normalize(glm::cross(cam.getFront(), cam.getUp())) * camSpeed *
  //       dt;
  //   if (!glm::all(glm::isnan(camPos))) {
  //     cam.setPosition(camPos);
  //   }
  // }
  // if (m_keys.at(KEY::W)) {
  //   glm::vec3 camPos = cam.getPosition() + cam.getFront() * camSpeed * dt;
  //   if (!glm::all(glm::isnan(camPos))) {
  //     cam.setPosition(camPos);
  //   }
  // }
  // if (m_keys.at(KEY::S)) {
  //   glm::vec3 camPos = cam.getPosition() - cam.getFront() * camSpeed * dt;
  //   if (!glm::all(glm::isnan(camPos))) {
  //     cam.setPosition(camPos);
  //   }
  // }
  // if (m_keys.at(KEY::Space)) {
  //   glm::vec3 camPos = cam.getPosition() + glm::vec3(0, 1, 0) * camSpeed *
  //   dt; if (!glm::all(glm::isnan(camPos))) {
  //     cam.setPosition(camPos);
  //   }
  // }

  if (m_keys.at(KEY::O)) {
    ecsManager.setSimulatePhysics(ecsManager.getSimulatePhysics() ? false
                                                                  : true);
  }

  // Toggle debug drawing when T is pressed
  static bool tWasPressed = false;
  if (m_keys.at(KEY::T) && !tWasPressed) {
    PhysicsSystem::getInstance().toggleDebugDrawing();
    tWasPressed = true;
  } else if (!m_keys.at(KEY::T)) {
    tWasPressed = false;
  }
  if (ecsManager.getSimulatePhysics()) {
    return;
  }

  static bool pressed = true;
  if (m_keys.at(KEY::Mouse1)) {
    glm::vec3 direction;
    direction.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    direction.y = sin(glm::radians(m_pitch));
    direction.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
    glm::vec3 cameraFront = glm::normalize(direction);
    auto cam = static_pointer_cast<CameraComponent>(
      ECSManager::getInstance().getCamera());
    cam->m_front = cameraFront;
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
    return;
  }

  double xoffset = x - lastX;
  double yoffset = lastY - y;
  lastX = x;
  lastY = y;

  float sensitivity = 0.1f;
  xoffset *= sensitivity;
  yoffset *= sensitivity;

  m_yaw += xoffset;
  m_pitch += yoffset;

  if (m_pitch > 89.0f)
    m_pitch = 89.0f;
  if (m_pitch < -89.0f)
    m_pitch = -89.0f;
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
