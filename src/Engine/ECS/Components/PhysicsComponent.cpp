#include "PhysicsComponent.hpp"

#include <ECS/Systems/PhysicsSystem.hpp>

PhysicsComponent::PhysicsComponent(Entity entity,
                                   float mass,
                                   CollisionShapeType type)
  : m_shapeType(type)
  , m_mass(mass)
  , m_entity(entity)
{
  m_bodyId = PhysicsSystem::getInstance().createBody(entity, mass, type);
}

PhysicsComponent&
PhysicsComponent::operator=(PhysicsComponent&& other) noexcept
{
  if (this != &other) {
    if (!m_bodyId.IsInvalid()) {
      PhysicsSystem::getInstance().destroyBody(m_bodyId);
    }
    m_bodyId = other.m_bodyId;
    m_shapeType = other.m_shapeType;
    m_mass = other.m_mass;
    m_entity = other.m_entity;
    other.m_bodyId = JPH::BodyID();
  }
  return *this;
}

PhysicsComponent::~PhysicsComponent()
{
  if (!m_bodyId.IsInvalid()) {
    PhysicsSystem::getInstance().destroyBody(m_bodyId);
  }
}
