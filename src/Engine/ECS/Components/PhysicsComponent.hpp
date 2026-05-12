#ifndef PHYSICSCOMPONENT_H_
#define PHYSICSCOMPONENT_H_

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

struct PhysicsComponent
{
  PhysicsComponent() = delete;
  PhysicsComponent(Entity entity, float mass, CollisionShapeType type);
  ~PhysicsComponent();

  PhysicsComponent(PhysicsComponent&& other) noexcept
    : m_bodyId(other.m_bodyId)
    , m_shapeType(other.m_shapeType)
    , m_mass(other.m_mass)
    , m_entity(other.m_entity)
  {
    other.m_bodyId = JPH::BodyID(); // Invalidate source so its dtor is a no-op
  }

  PhysicsComponent& operator=(PhysicsComponent&& other) noexcept;

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
