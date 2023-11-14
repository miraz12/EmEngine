#include "Game.hpp"
#include "SceneLoader.hpp"

Game::Game(GLFWwindow &window)
    : m_ECSManager(ECSManager::getInstance()), m_window(window),
      m_InputManager(InputManager::getInstance()) {
  SceneLoader::getInstance().init("resources/scene.yaml");
  SceneLoader::getInstance().saveScene("resources/sceneOUT.yaml");
}

void Game::update(float dt) {
  m_InputManager.update(dt);
  m_ECSManager.updateDirLight(dirLightColor, dirLightAmbient, dirLightDir);
  m_ECSManager.dDir = dirLightDir;
  m_ECSManager.update(dt);
}

void Game::setViewport(u32 w, u32 h) {
  m_ECSManager.setViewport(w, h);
}

