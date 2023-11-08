#ifndef POSITIONSYSTEM_H_
#define POSITIONSYSTEM_H_

#include "System.hpp"
#include <Singleton.hpp>

class PositionSystem : public System, public Singleton<PositionSystem> {
  friend class Singleton<PositionSystem>;

public:
  void update(float dt) override;
  void setViewport(u32 /* w */, u32 /* h */){};

private:
  PositionSystem() = default;
};

#endif // POSITIONSYSTEM_H_
