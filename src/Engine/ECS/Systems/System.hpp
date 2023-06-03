#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <Singleton.hpp>
#include <vector>

#include "../Components/Component.hpp"
#include "../Entity.hpp"

class ECSManager;
class System {

public:
  // this function is called when the system should do its thing
  virtual void update(float dt) = 0;
  // Initialize system
  virtual void initialize(ECSManager &ecsManager) { m_manager = &ecsManager; };

protected:
  System() = default;
  virtual ~System() = default;

  ECSManager *m_manager;
};
#endif // SYSTEM_H_
