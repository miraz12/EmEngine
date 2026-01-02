#include "GameStateManager.hpp"
#include "UIManager.hpp"
#include <iostream>

void
GameStateManager::initialize()
{
  m_uiManager = &UIManager::getInstance();
  std::cout << "[GameStateManager] Initialized\n";

  // Start in main menu state
  enterState(GameState::MainMenu);
}

void
GameStateManager::changeState(GameState newState)
{
  if (m_currentState == newState) {
    return;
  }

  std::cout << "[GameStateManager] Changing state from "
            << static_cast<int>(m_currentState) << " to "
            << static_cast<int>(newState) << "\n";

  exitState(m_currentState);
  m_currentState = newState;
  enterState(m_currentState);
}

void
GameStateManager::startGame()
{
  changeState(GameState::Playing);
}

void
GameStateManager::pauseGame()
{
  if (m_currentState == GameState::Playing) {
    changeState(GameState::Paused);
  }
}

void
GameStateManager::resumeGame()
{
  if (m_currentState == GameState::Paused) {
    changeState(GameState::Playing);
  }
}

void
GameStateManager::returnToMainMenu()
{
  changeState(GameState::MainMenu);
}

void
GameStateManager::gameOver()
{
  changeState(GameState::GameOver);
}

void
GameStateManager::onEnterState(GameState state, StateCallback callback)
{
  m_onEnterCallbacks[state].push_back(callback);
}

void
GameStateManager::onExitState(GameState state, StateCallback callback)
{
  m_onExitCallbacks[state].push_back(callback);
}

void
GameStateManager::enterState(GameState state)
{
  // Execute registered callbacks
  auto it = m_onEnterCallbacks.find(state);
  if (it != m_onEnterCallbacks.end()) {
    for (const auto& callback : it->second) {
      callback();
    }
  }

  // Setup UI for the new state
  switch (state) {
    case GameState::MainMenu:
      setupMainMenuUI();
      break;
    case GameState::Playing:
      setupPlayingUI();
      break;
    case GameState::Paused:
      setupPausedUI();
      break;
    case GameState::GameOver:
      setupGameOverUI();
      break;
  }
}

void
GameStateManager::exitState(GameState state)
{
  // Execute registered callbacks
  auto it = m_onExitCallbacks.find(state);
  if (it != m_onExitCallbacks.end()) {
    for (const auto& callback : it->second) {
      callback();
    }
  }

  // Hide UI for the exiting state
  switch (state) {
    case GameState::MainMenu:
      if (m_uiManager->isDocumentVisible("main_menu")) {
        m_uiManager->hideDocument("main_menu");
      }
      break;
    case GameState::Playing:
      // No UI to hide in playing state
      break;
    case GameState::Paused:
      if (m_uiManager->isDocumentVisible("pause_menu")) {
        m_uiManager->hideDocument("pause_menu");
      }
      break;
    case GameState::GameOver:
      if (m_uiManager->isDocumentVisible("game_over")) {
        m_uiManager->hideDocument("game_over");
      }
      break;
  }
}

void
GameStateManager::setupMainMenuUI()
{
  std::cout << "[GameStateManager] Setting up Main Menu UI\n";

  if (!m_uiManager->isDocumentVisible("main_menu")) {
    if (!m_uiManager->loadDocument("main_menu.rml", "main_menu")) {
      std::cerr << "[GameStateManager] Failed to load main menu\n";
      return;
    }

    m_uiManager->registerClickCallback("main_menu", "btn-play", [this]() {
      std::cout << "[GameStateManager] Play button clicked!\n";
      startGame();
    });

    m_uiManager->registerClickCallback("main_menu", "btn-settings", [this]() {
      std::cout << "[GameStateManager] Settings button clicked!\n";
      // TODO: Implement settings menu
    });

    m_uiManager->registerClickCallback("main_menu", "btn-quit", [this]() {
      std::cout << "[GameStateManager] Quit button clicked!\n";
      // TODO: Implement quit functionality
    });
  }

  // Show the main menu
  m_uiManager->showDocument("main_menu");
}

void
GameStateManager::setupPlayingUI()
{
  std::cout << "[GameStateManager] Setting up Playing UI\n";
}

void
GameStateManager::setupPausedUI()
{
  std::cout << "[GameStateManager] Setting up Paused UI\n";

  if (!m_uiManager->loadDocument("pause_menu.rml", "pause_menu")) {
    std::cout << "[GameStateManager] Pause menu not found, showing main menu\n";
    m_uiManager->showDocument("main_menu");
  } else {
    m_uiManager->showDocument("pause_menu");
  }
}

void
GameStateManager::setupGameOverUI()
{
  std::cout << "[GameStateManager] Setting up Game Over UI\n";

  // Load and show game over screen
  if (!m_uiManager->isDocumentVisible("game_over")) {
    if (!m_uiManager->loadDocument("game_over.rml", "game_over")) {
      std::cout << "[GameStateManager] Game over screen not found\n";
    } else {
      m_uiManager->showDocument("game_over");
    }
  }
}
