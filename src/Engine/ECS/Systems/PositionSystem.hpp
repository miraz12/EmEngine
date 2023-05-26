#ifndef POSITIONSYSTEM_H_
#define POSITIONSYSTEM_H_

#include <Camera.hpp>
#include <ECS/Systems/System.hpp>
class PositionSystem : public System {
public:
  PositionSystem(ECSManager &ECSManager);
  ~PositionSystem(){};
  void update(float dt);
  void setViewport(uint32_t /* w */, uint32_t /* h */){};
};

#endif // POSITIONSYSTEM_H_
