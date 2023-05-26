#ifndef PHYSICSCOMPONENT_H_
#define PHYSICSCOMPONENT_H_

#include "Component.hpp"
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btSphereShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ECS/ECSManager.hpp>

class PhysicsComponent : public Component {
public:
  PhysicsComponent(ECSManager &ECSManager);
  PhysicsComponent() = delete;
  virtual ~PhysicsComponent(){};
  btRigidBody *getRigidBody() { return body; }

private:
  btCollisionShape *colShape;
  btRigidBody *body;
};

#endif // PHYSICSCOMPONENT_H_
