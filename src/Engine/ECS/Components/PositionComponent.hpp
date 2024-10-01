#ifndef POSITIONCOMPONENT_H_
#define POSITIONCOMPONENT_H_

#include "Component.hpp"

class PositionComponent : public Component {
public:
  glm::vec3 position{0.0f};
  glm::quat rotation{glm::identity<glm::quat>()};
  glm::vec3 scale{1.0f};
  glm::mat4 model;

  PositionComponent() = default;
  PositionComponent(float startX, float startY)
      : position(startX, startY, -0.1f) {}
};

#endif // POSITIONCOMPONENT_H_
