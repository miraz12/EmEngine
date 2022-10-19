#define NOMINMAX
#include "Game.hpp"

#include <algorithm>

Game::Game(GLFWwindow *window)
    : m_window(window), m_ECSManager(&ECSManager::getInstance()) {
  m_ECSManager->createPlayerEntity(0, 0, m_window);
}

void Game::update(float dt) { m_ECSManager->update(dt); }

Game::~Game() {}
