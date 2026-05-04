#include "PhysicsComponent.hpp"

#include <ECS/Systems/PhysicsSystem.hpp>

PhysicsComponent::PhysicsComponent(Entity entity,
                                   float mass,
                                   CollisionShapeType type)
  : m_entity(entity)
  , m_shapeType(type)
  , m_mass(mass)
{
  m_bodyId = PhysicsSystem::getInstance().createBody(entity, mass, type);
}

PhysicsComponent::~PhysicsComponent()
{
  if (!m_bodyId.IsInvalid()) {
    PhysicsSystem::getInstance().destroyBody(m_bodyId);
  }
}
