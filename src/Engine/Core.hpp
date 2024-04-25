#ifndef CORE_H_
#define CORE_H_
#include "Window.hpp"
#include <ECS/ECSManager.hpp>

class Core : public Singleton<Core> {
  friend class Singleton<Core>;

public:
  Core() = default;
  ~Core() = default;

  bool initialize();
  static void update();
  bool open();
  float &getDeltaTime();

private:
  ECSManager *m_ECSManager;
  float m_dt, m_prevTime, m_currentTime;
};

#endif // CORE_H_