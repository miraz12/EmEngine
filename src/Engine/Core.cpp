#include "Core.hpp"
#include "InputManager.hpp"
#include "Window.hpp"
#include "engine_api.hpp"
#ifndef NDEBUG
#include "ECS/Systems/GraphicsSystem.hpp"
#endif

namespace {
u32 pendingW = 0;
u32 pendingH = 0;
bool resizePending = false;
}

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
      if (width > 0 && height > 0) {
        pendingW = static_cast<u32>(width);
        pendingH = static_cast<u32>(height);
        resizePending = true;
      }
    });

  int width, height;
  glfwGetFramebufferSize(Window::getInstance().getWindow(), &width, &height);
  if (width > 0 && height > 0) {
    pendingW = static_cast<u32>(width);
    pendingH = static_cast<u32>(height);
    resizePending = true;
  }

  return true;
}

void
Core::updateDeltaTime()
{
  m_currentTime = glfwGetTime();
  m_dt =
    std::min(static_cast<float>(m_currentTime - m_prevTime), kMaxDeltaTime);
  m_prevTime = m_currentTime;
}

void
Core::update()
{
  glfwPollEvents();
  updateDeltaTime();

#ifndef NDEBUG
  m_profiler.beginFrame();
  m_profiler.resetSections();
#endif

  // Update UI
#ifndef NDEBUG
  m_profiler.beginPhase(Profiler::kPhaseUI);
#endif
  m_UIManager->update(m_dt);
#ifndef NDEBUG
  m_profiler.endPhase(Profiler::kPhaseUI);
#endif

  // Update game logic (ECS systems including rendering)
#ifndef NDEBUG
  m_profiler.beginPhase(Profiler::kPhaseECS);
#endif
  InputManager::getInstance().update(m_dt);
  ECSManager::getInstance().update(m_dt);
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

  if (resizePending) {
    resizePending = false;
    ECSManager::getInstance().setViewport(pendingW, pendingH);
    UIManager::getInstance().onResize(static_cast<int>(pendingW),
                                      static_cast<int>(pendingH));
  }
}

bool
Core::open()
{
  return !Window::getInstance().closed();
}
