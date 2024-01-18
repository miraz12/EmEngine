#include "Game.hpp"

Game::Game(GLFWwindow &window)
    : m_ECSManager(API::ECS::getECS()), m_window(window),
      m_inputManager(*new InputManager()) {
  m_ECSManager.loadScene("resources/scene.yaml");
}

void Game::update(float dt) {
  m_inputManager.update(dt);
  // m_ECSManager.updateDirLight(dirLightColor, dirLightAmbient, dirLightDir);
  // m_ECSManager.dDir = dirLightDir;
  m_ECSManager.update(dt);
}

void Game::setViewport(u32 w, u32 h) { m_ECSManager.setViewport(w, h); }
