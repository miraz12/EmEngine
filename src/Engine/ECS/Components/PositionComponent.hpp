#ifndef POSITIONCOMPONENT_H_
#define POSITIONCOMPONENT_H_

#include <glm/glm.hpp>

#include "Component.hpp"

class PositionComponent : public Component<PositionComponent> {
public:
  glm::vec3 position;
  float rotation;
  glm::vec3 scale;

  PositionComponent();
  PositionComponent(float startX, float startY);

  glm::mat4 calculateMatrix();
};

#endif // POSITIONCOMPONENT_H_
