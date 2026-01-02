#pragma once

#include "Singleton.hpp"
#include <functional>
#include <string>
#include <unordered_map>

// Forward declarations
class UIManager;

enum class GameState
{
  MainMenu,
  Playing,
  Paused,
  GameOver
};

class GameStateManager : public Singleton<GameStateManager>
{
  friend class Singleton<GameStateManager>;

public:
  using StateCallback = std::function<void()>;

  // Initialize the state manager
  void initialize();

  // State transitions
  void changeState(GameState newState);
  void startGame();
  void pauseGame();
  void resumeGame();
  void returnToMainMenu();
  void gameOver();

  // Get current state
  GameState getCurrentState() const { return m_currentState; }
  bool isPlaying() const { return m_currentState == GameState::Playing; }
  bool isPaused() const { return m_currentState == GameState::Paused; }

  // Register callbacks for state changes
  void onEnterState(GameState state, StateCallback callback);
  void onExitState(GameState state, StateCallback callback);

private:
  GameStateManager() = default;
  ~GameStateManager() = default;

  void enterState(GameState state);
  void exitState(GameState state);

  // UI management for each state
  void setupMainMenuUI();
  void setupPlayingUI();
  void setupPausedUI();
  void setupGameOverUI();

  GameState m_currentState = GameState::MainMenu;
  UIManager* m_uiManager = nullptr;

  // Callbacks for state transitions
  std::unordered_map<GameState, std::vector<StateCallback>> m_onEnterCallbacks;
  std::unordered_map<GameState, std::vector<StateCallback>> m_onExitCallbacks;
};
