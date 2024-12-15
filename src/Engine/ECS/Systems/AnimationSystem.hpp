#ifndef ANIMATIONSYSTEM_H_
#define ANIMATIONSYSTEM_H_

#include "System.hpp"
#include <Singleton.hpp>

class AnimationSystem final
  : public System
  , public Singleton<AnimationSystem>
{
  friend class Singleton<AnimationSystem>;

public:
  void update(float dt) override;

private:
  AnimationSystem() = default;
  ~AnimationSystem() override = default;

  void scale(size_t index, float time, float scale);
  void rotate(size_t index, float time, glm::quat rot);
  void translate(size_t index, float time, glm::vec3 trans);
};
#endif // ANIMATIONSYSTEM_H_
