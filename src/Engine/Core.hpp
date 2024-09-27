#ifndef CORE_H_
#define CORE_H_
#include <ECS/ECSManager.hpp>
#include <Gui.hpp>

class Core : public Singleton<Core> {
  friend class Singleton<Core>;

public:
  Core() = default;
  ~Core() = default;

  bool initialize();
  void update();
  bool open();
  float &getDeltaTime();

private:
  ECSManager *m_ECSManager;
  float m_dt;
  float m_prevTime;
  float m_currentTime;

  GUI m_gui;
};

#endif // CORE_H_
