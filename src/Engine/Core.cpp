#include "Core.hpp"
#include "InputManager.hpp"
#include "Window.hpp"
#include "engine_api.hpp"

bool
Core::initialize()
{
  std::cout << "[core] Initialize" << std::endl;

  if (Window::getInstance().start() && Window::getInstance().open()) {
    m_ECSManager = &ECSManager::getInstance();
    m_ECSManager->initializeSystems();
    m_prevTime = glfwGetTime();
  } else {
    return false;
  }

  Window::getInstance().setCursorPosCallback(
    [](GLFWwindow* /* win */, double xpos, double ypos) {
      InputManager::getInstance().setMousePos(xpos, ypos);
      ImGuiIO& io = ImGui::GetIO();
      io.AddMousePosEvent(xpos, ypos);
    });
  Window::getInstance().setMouseButtonCallback(
    [](GLFWwindow* /* win */, i32 button, i32 action, i32 /* mods */) {
      ImGuiIO& io = ImGui::GetIO();
      io.AddMouseButtonEvent(button, action);
      if (!io.WantCaptureMouse) {
        InputManager::getInstance().handleInput(button, action);
      }
    });
  Window::getInstance().setKeyCallback([](GLFWwindow* /* win */,
                                          i32 key,
                                          i32 /* scancode */,
                                          i32 action,
                                          i32 /* mods */) {
    if (key == GLFW_KEY_ESCAPE) {
      Window::getInstance().close();
    }
    InputManager::getInstance().handleInput(key, action);
  });
  Window::getInstance().setFramebufferSizeCallback(
    [](GLFWwindow* /* win */, i32 width, i32 height) {
      ECSManager::getInstance().setViewport(width, height);
    });

  return true;
}

float&
Core::getDeltaTime()
{
  m_currentTime = glfwGetTime();
  m_dt = m_currentTime - m_prevTime;
  m_prevTime = m_currentTime;

  static double lastFPSPrintTime = 0.0;
  // Check if 3 seconds have passed since the last FPS print
  if (m_currentTime - lastFPSPrintTime >= 3.0) {
    // Calculate and print FPS
    float fps = 1.0f / m_dt;
    std::cout << "FPS: " << fps << std::endl;

    // Update the last FPS print time
    lastFPSPrintTime = m_currentTime;
  }
  return m_dt;
}

void
Core::update()
{
  glfwPollEvents();

  m_gui.renderGUI();
  float& dt = getDeltaTime();
  InputManager::getInstance().update(dt);
  ECSManager::getInstance().update(dt);
  Window::getInstance().swap();
}

bool
Core::open()
{
  return !Window::getInstance().closed();
}
