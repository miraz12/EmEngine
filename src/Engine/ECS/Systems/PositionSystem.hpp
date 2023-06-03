#ifndef POSITIONSYSTEM_H_
#define POSITIONSYSTEM_H_

#include <Camera.hpp>
#include <ECS/Systems/System.hpp>
#include <Singleton.hpp>

class PositionSystem : public System, public Singleton<PositionSystem> {
  friend class Singleton<PositionSystem>;

public:
  void update(float dt) override;
  void setViewport(uint32_t /* w */, uint32_t /* h */){};

private:
  PositionSystem() = default;
};

#endif // POSITIONSYSTEM_H_
