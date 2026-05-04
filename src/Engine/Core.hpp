#ifndef CORE_H_
#define CORE_H_
#include <ECS/ECSManager.hpp>
#include <GameStateManager.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
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
  ECSManager* m_ECSManager = nullptr;
  UIManager* m_UIManager = nullptr;
  GameStateManager* m_gameStateManager = nullptr;
  float m_dt = 0.0f;
  float m_prevTime = 0.0f;
  float m_currentTime = 0.0f;

  GUI m_gui;
};

#endif // CORE_H_
