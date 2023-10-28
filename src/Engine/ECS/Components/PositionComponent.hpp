#ifndef POSITIONCOMPONENT_H_
#define POSITIONCOMPONENT_H_


#include "Component.hpp"

class PositionComponent : public Component {
public:
  glm::vec3 position;
  glm::quat rotation;
  glm::vec3 scale;
  glm::mat4 model;

  PositionComponent()
      : position(0.0f, 0.0f, 0.0f), rotation(glm::identity<glm::quat>()), scale(1.0f) {}
  PositionComponent(float startX, float startY)
      : position(startX, startY, -0.1f), rotation(glm::identity<glm::quat>()), scale(1.0f) {}
};

#endif // POSITIONCOMPONENT_H_
