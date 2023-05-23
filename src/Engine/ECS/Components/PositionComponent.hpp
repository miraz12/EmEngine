#ifndef POSITIONCOMPONENT_H_
#define POSITIONCOMPONENT_H_

#include <glm/glm.hpp>

#include "Component.hpp"
#include "glm/fwd.hpp"

class PositionComponent : public Component {
public:
  glm::vec3 position;
  float rotation;
  glm::vec3 scale;
  glm::mat4 model;

  PositionComponent();
  PositionComponent(float startX, float startY);

};

#endif // POSITIONCOMPONENT_H_
