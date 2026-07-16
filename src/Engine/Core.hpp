#ifndef CORE_H_
#define CORE_H_
#include <ECS/ECSManager.hpp>
#include <GameStateManager.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#ifndef NDEBUG
#include <Gui.hpp>
#include <Profiler.hpp>
#endif
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
  void updateDeltaTime();
  float getDt() const { return m_dt; }

private:
  ECSManager* m_ECSManager = nullptr;
  UIManager* m_UIManager = nullptr;
  GameStateManager* m_gameStateManager = nullptr;
  static constexpr float kMaxDeltaTime = 0.25f;
  double m_prevTime = 0.0;
  double m_currentTime = 0.0;
  float m_dt = 0.0f;

#ifndef NDEBUG
  GUI m_gui;
  Profiler m_profiler;
#endif
};

#endif // CORE_H_
