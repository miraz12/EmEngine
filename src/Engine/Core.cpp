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

    // Initialize UIManager
    m_UIManager = &UIManager::getInstance();
    int width, height;
    glfwGetFramebufferSize(Window::getInstance().getWindow(), &width, &height);
    if (!m_UIManager->initialize(
          Window::getInstance().getWindow(), width, height)) {
      std::cerr << "Failed to initialize UIManager" << std::endl;
      return false;
    }

    // Initialize GameStateManager
    m_gameStateManager = &GameStateManager::getInstance();
    m_gameStateManager->initialize();

    m_prevTime = glfwGetTime();
  } else {
    return false;
  }

  Window::getInstance().setCursorPosCallback(
    [](GLFWwindow* /* win */, double xpos, double ypos) {
      InputManager::getInstance().setMousePos(xpos, ypos);
      ImGuiIO& io = ImGui::GetIO();
      io.AddMousePosEvent(xpos, ypos);
      UIManager::getInstance().onMouseMove(
        static_cast<int>(xpos), static_cast<int>(ypos), 0);
    });
  Window::getInstance().setMouseButtonCallback(
    [](GLFWwindow* /* win */, i32 button, i32 action, i32 /* mods */) {
      ImGuiIO& io = ImGui::GetIO();
      io.AddMouseButtonEvent(button, action);
      UIManager::getInstance().onMouseButton(button, action, 0);
      if (!io.WantCaptureMouse &&
          !UIManager::getInstance().wantsMouseCapture()) {
        InputManager::getInstance().handleInput(button, action);
      }
    });
  Window::getInstance().setKeyCallback([](GLFWwindow* /* win */,
                                          i32 key,
                                          i32 /* scancode */,
                                          i32 action,
                                          i32 mods) {
    if (key == GLFW_KEY_ESCAPE) {
      Window::getInstance().close();
    }
    UIManager::getInstance().onKey(key, action, mods);
    if (!UIManager::getInstance().wantsKeyboardCapture()) {
      InputManager::getInstance().handleInput(key, action);
    }
  });
  Window::getInstance().setFramebufferSizeCallback(
    [](GLFWwindow* /* win */, i32 width, i32 height) {
      ECSManager::getInstance().setViewport(width, height);
      UIManager::getInstance().onResize(width, height);
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

  float& dt = getDeltaTime();

  // Update UI
  m_UIManager->update(dt);

  // Update game logic
  InputManager::getInstance().update(dt);
  ECSManager::getInstance().update(dt);

  // Render debug GUI and UI
  m_gui.renderGUI();
  m_UIManager->render();

  Window::getInstance().swap();
}

bool
Core::open()
{
  return !Window::getInstance().closed();
}
