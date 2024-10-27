#ifndef PHYSICSCOMPONENT_H_
#define PHYSICSCOMPONENT_H_

#include "Component.hpp"
#include "GraphicsComponent.hpp"
#include "PositionComponent.hpp"

enum class CollisionShapeType { BOX, SPHERE, CAPSULE, CONVEX_HULL };

class PhysicsComponent final : public Component {
public:
  PhysicsComponent() = delete;
  // Create colision mesh from grapComp and give position
  PhysicsComponent(std::size_t en, float mass, CollisionShapeType type);
  ~PhysicsComponent() override;

  btRigidBody *getRigidBody() { return body; }
  btScalar getMass() { return mass; }

private:
  void setupConvexHub();

public:
  btTransform startTransform;
  btVector3 initialPos{0., 0., 0.};
  btVector3 initialScale{1., 1., 1.};
  btQuaternion initialRotation{1., 1., 1., 1.};
  btDefaultMotionState *myMotionState;
  std::size_t m_en;

  btRigidBody *body;            // Bullet physics body
  btCollisionShape *shape;      // Bullet collision shape
  CollisionShapeType shapeType; // Type of collision shape
  btScalar mass;                // Mass of the object
  btVector3 dimensions;         // General dimensions (for box and sphere)
  float capsuleRadius;          // Radius for capsule shape
  float capsuleHeight;          // Height for capsule shape
  btTriangleMesh *mesh;         // Optional mesh (for complex shapes)
};

#endif // PHYSICSCOMPONENT_H_
