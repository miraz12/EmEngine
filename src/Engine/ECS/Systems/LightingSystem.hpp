#ifndef LIGHTINGSYSTEM_H_
#define LIGHTINGSYSTEM_H_

#include "System.hpp"

class LightingSystem : public System {
 public:
  LightingSystem(ECSManager *ECSManager);
  void update(float dt);
};

#endif  // LIGHTINGSYSTEM_H_
