#include "PositionComponent.hpp"

#include <glm/ext.hpp>

PositionComponent::PositionComponent() : position(0.0f, 0.0f, 0.0f), rotation(glm::identity<glm::quat>()), scale(1.0f) {}

PositionComponent::PositionComponent(float startX, float startY)
    : position(startX, startY, -0.1f), rotation(glm::identity<glm::quat>()), scale(1.0f) {}
