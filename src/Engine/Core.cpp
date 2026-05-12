#include "Core.hpp"
#include "InputManager.hpp"
#include "Window.hpp"
#include "engine_api.hpp"
#ifndef NDEBUG
#include "ECS/Systems/GraphicsSystem.hpp"
#endif

bool
Core::initialize()
{
  std::cout << "[core] Initialize" << std::endl;

  if (Window::getInstance().start() && Window::getInstance().open()) {
    // Initialize GraphicsDevice (must be after GL context is created)
    if (!gfx::GraphicsDevice::getInstance().initialize()) {
      std::cerr << "Failed to initialize GraphicsDevice" << std::endl;
      return false;
    }

    // Initialize RenderResources bridge layer
    if (!gfx::RenderResources::getInstance().initialize()) {
      std::cerr << "Failed to initialize RenderResources" << std::endl;
      return false;
    }

    m_ECSManager = &ECSManager::getInstance();
    m_ECSManager->initializeSystems();
#ifndef NDEBUG
    m_ECSManager->setProfiler(&m_profiler);
    static_cast<GraphicsSystem&>(m_ECSManager->getSystem("GRAPHICS"))
      .setProfiler(&m_profiler);
#endif

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
#ifndef NDEBUG
      ImGuiIO& io = ImGui::GetIO();
      io.AddMousePosEvent(xpos, ypos);
#endif
      UIManager::getInstance().onMouseMove(
        static_cast<int>(xpos), static_cast<int>(ypos), 0);
    });
  Window::getInstance().setMouseButtonCallback(
    [](GLFWwindow* /* win */, i32 button, i32 action, i32 /* mods */) {
#ifndef NDEBUG
      ImGuiIO& io = ImGui::GetIO();
      io.AddMouseButtonEvent(button, action);
#endif
      UIManager::getInstance().onMouseButton(button, action, 0);
#ifndef NDEBUG
      if (!io.WantCaptureMouse &&
          !UIManager::getInstance().wantsMouseCapture()) {
        InputManager::getInstance().handleInput(button, action);
      }
#else
      if (!UIManager::getInstance().wantsMouseCapture()) {
        InputManager::getInstance().handleInput(button, action);
      }
#endif
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
  return m_dt;
}

void
Core::update()
{
  glfwPollEvents();

  float& dt = getDeltaTime();

#ifndef NDEBUG
  m_profiler.beginFrame();
  m_profiler.resetSections();
#endif

  // Update UI
#ifndef NDEBUG
  m_profiler.beginPhase(Profiler::kPhaseUI);
#endif
  m_UIManager->update(dt);
#ifndef NDEBUG
  m_profiler.endPhase(Profiler::kPhaseUI);
#endif

  // Update game logic (ECS systems including rendering)
#ifndef NDEBUG
  m_profiler.beginPhase(Profiler::kPhaseECS);
#endif
  InputManager::getInstance().update(dt);
  ECSManager::getInstance().update(dt);
#ifndef NDEBUG
  m_profiler.endPhase(Profiler::kPhaseECS);
#endif

  // Render debug GUI and UI
#ifndef NDEBUG
  m_gui.renderGUI(m_profiler);
#endif
  m_UIManager->render();

#ifndef NDEBUG
  m_profiler.beginPhase(Profiler::kPhaseSwap);
#endif
  Window::getInstance().swap();
#ifndef NDEBUG
  m_profiler.endPhase(Profiler::kPhaseSwap);
  m_profiler.endFrame();
#endif
}

bool
Core::open()
{
  return !Window::getInstance().closed();
}
