#ifndef CORE_H_
#define CORE_H_
#include <ECS/ECSManager.hpp>
#include <GameStateManager.hpp>
#include <Gui.hpp>
#include <UIManager.hpp>

class Core : public Singleton<Core>
{
  friend class Singleton<Core>;

public:
  Core() = default;
  ~Core() = default;

  bool initialize();
  void update();
  bool open();
  float& getDeltaTime();

private:
  ECSManager* m_ECSManager;
  UIManager* m_UIManager;
  GameStateManager* m_gameStateManager;
  float m_dt;
  float m_prevTime;
  float m_currentTime;

  GUI m_gui;
};

#endif // CORE_H_
