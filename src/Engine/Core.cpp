#include "Core.hpp"
#include "InputManager.hpp"
#include <game_api.hpp>

bool Core::initialize() {
  std::cout << "[core] Initialize" << std::endl;
  if (Window::getInstance().start() && Window::getInstance().open()) {
    m_ECSManager = &ECSManager::getInstance();

    game_initialize();
  }

  Window::getInstance().setCursorPosCallback(
      [](GLFWwindow * /* win */, double xpos, double ypos) -> void {
        InputManager::getInstance().setMousePos(xpos, ypos);
      });
  Window::getInstance().setMouseButtonCallback([](GLFWwindow * /* win */,
                                                  i32 button, i32 action,
                                                  i32 /* mods */) -> void {
    InputManager::getInstance().handleInput(button, action);
  });
  Window::getInstance().setKeyCallback([](GLFWwindow *win, i32 key,
                                          i32 /* scancode */, i32 action,
                                          i32 /* mods */) -> void {
    if (key == GLFW_KEY_ESCAPE) {
      glfwSetWindowShouldClose(win, true);
    }
    InputManager::getInstance().handleInput(key, action);
  });
  Window::getInstance().setFramebufferSizeCallback(
      [](GLFWwindow * /* win */, i32 width, i32 height) -> void {
        ECSManager::getInstance().setViewport(width, height);
      });

  return true;
}

void Core::update(float dt) {
  glfwPollEvents();

  ECSManager::getInstance().update(dt);
  game_update(dt);

  Window::getInstance().swap();
}

bool Core::open() { return !Window::getInstance().closed(); }
