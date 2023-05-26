#ifndef SYSTEM_H_
#define SYSTEM_H_

#include <vector>

#include "../Components/Component.hpp"
#include "../Entity.hpp"

class ECSManager;
class System {
public:
  // Created many different constructors for different amount of required
  // components

  System(ECSManager &ECSManager);

  virtual ~System() = default;

  // this function is called when the system should do its thing
  virtual void update(float dt) = 0;

  // Initialize system
  virtual void initialize(){};

protected:
  ECSManager &m_manager;
};
#endif // SYSTEM_H_
