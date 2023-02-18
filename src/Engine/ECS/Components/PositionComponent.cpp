#include "PositionComponent.hpp"

#include <glm/ext.hpp>

PositionComponent::PositionComponent() : position(0.0f, 0.0f, 0.0f), rotation(0.0f), scale(1.0f) {
  m_componentType = ComponentTypeEnum::POSITION;
}

PositionComponent::PositionComponent(float startX, float startY)
    : position(startX, startY, -0.1f), rotation(0.0f), scale(1.0f) {
  m_componentType = ComponentTypeEnum::POSITION;
}

glm::mat4 PositionComponent::calculateMatrix() {
  glm::mat4 tempMatrix = glm::scale(glm::identity<glm::mat4>(), scale);
  tempMatrix = glm::rotate(tempMatrix, rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
  tempMatrix = glm::translate(tempMatrix, position);
  return tempMatrix;
}
