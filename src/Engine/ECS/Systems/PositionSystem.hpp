#ifndef POSITIONSYSTEM_H_
#define POSITIONSYSTEM_H_

#include <Camera.hpp>
#include <ECS/Systems/System.hpp>
class PositionSystem : public System {
public:
  PositionSystem(ECSManager *ECSManager, Camera &cam);
  ~PositionSystem(){};
  void update(float dt);
  void setViewport(unsigned int w, unsigned int h){};
};

#endif // POSITIONSYSTEM_H_