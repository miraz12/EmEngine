#include "GraphicsComponent.hpp"

#include "ECS/ECSManager.hpp"

GraphicsComponent::GraphicsComponent(GraphicsObject &grapComp) : m_grapObj(grapComp) {}
GraphicsComponent::~GraphicsComponent() { delete &m_grapObj; }
