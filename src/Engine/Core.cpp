#include "Core.hpp"
#include "InputManager.hpp"

bool Core::initialize() {
  std::cout << "[core] Initialize" << std::endl;
  if (m_window.start() && m_window.open()) {
    m_ECSManager = &ECSManager::getInstance();

    Game_initialize();

    // m_game = &API::Game::getGame();
  }

  m_window.setCursorPosCallback(
      [](GLFWwindow * /* win */, double xpos, double ypos) -> void {
        InputManager::getInstance().setMousePos(xpos, ypos);
      });
  m_window.setMouseButtonCallback([](GLFWwindow * /* win */, i32 button,
                                     i32 action, i32 /* mods */) -> void {
    InputManager::getInstance().handleInput(button, action);
  });
  m_window.setKeyCallback([](GLFWwindow *win, i32 key, i32 /* scancode */,
                             i32 action, i32 /* mods */) -> void {
    if (key == GLFW_KEY_ESCAPE) {
      glfwSetWindowShouldClose(win, true);
    }
    InputManager::getInstance().handleInput(key, action);
  });
  m_window.setFramebufferSizeCallback(
      [](GLFWwindow * /* win */, i32 width, i32 height) -> void {
        ECSManager::getInstance().setViewport(width, height);
      });

  return true;
}
void Core::update(float dt) {
  glfwPollEvents();

  m_ECSManager->update(dt);
  // m_game->update(dt);

  m_window.swap();
}

bool Core::open() { return !m_window.closed(); }
