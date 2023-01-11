#include "GraphicsComponent.hpp"

#include "ECS/ECSManager.hpp"

GraphicsComponent::GraphicsComponent() {
  m_componentType = ComponentTypeEnum::GRAPHICS;
}

GraphicsComponent::~GraphicsComponent() { delete grapObj; }
