#ifndef PHYSICSCOMPONENT_H_
#define PHYSICSCOMPONENT_H_

#include "Component.hpp"

#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>

enum class CollisionShapeType
{
  BOX,
  SPHERE,
  CAPSULE,
  CONVEX_HULL,
  HEIGHTMAP
};

class PhysicsComponent final : public Component
{
public:
  PhysicsComponent() = delete;
  PhysicsComponent(Entity entity, float mass, CollisionShapeType type);
  ~PhysicsComponent() override;

  JPH::BodyID getBodyID() const { return m_bodyId; }
  float getMass() const { return m_mass; }
  CollisionShapeType getShapeType() const { return m_shapeType; }
  bool isValid() const { return !m_bodyId.IsInvalid(); }

private:
  JPH::BodyID m_bodyId;
  CollisionShapeType m_shapeType;
  float m_mass;
  Entity m_entity;
};

#endif // PHYSICSCOMPONENT_H_
