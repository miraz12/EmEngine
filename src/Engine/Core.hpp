#ifndef CORE_H_
#define CORE_H_
#include "Window.hpp"
#include <ECS/ECSManager.hpp>
#include <game_api.hpp>

class Core : public Singleton<Core> {
  friend class Singleton<Core>;

public:
  Core() = default;
  ~Core() = default;

  bool initialize();
  void update(float dt);
  bool open();

private:
  Window m_window;
  ECSManager *m_ECSManager;
};

#endif // CORE_H_
