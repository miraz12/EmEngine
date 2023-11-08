#ifndef PHYSICSCOMPONENT_H_
#define PHYSICSCOMPONENT_H_

#include "Component.hpp"
#include "GraphicsComponent.hpp"
#include "PositionComponent.hpp"

class PhysicsComponent : public Component {
public:
  PhysicsComponent();
  // Create colision mesh from grapComp and give position
  PhysicsComponent(std::size_t en, float mass);
  ~PhysicsComponent();
  btRigidBody *getRigidBody() { return body; }
  bool initialized{false};
  // Used for delayed init as this component needs graphics component
  void init();

private:
  btRigidBody *body;
  btTransform startTransform;
  btVector3 initialPos{0., 0., 0.};
  btVector3 initialScale{1., 1., 1.};
  btQuaternion initialRotation{1., 1., 1., 1.};
  btScalar mass;
  btCollisionShape *colShape;
  btDefaultMotionState *myMotionState;
  std::size_t m_en;
};

#endif // PHYSICSCOMPONENT_H_
