#include "PhysicsComponent.hpp"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <LinearMath/btDefaultMotionState.h>

PhysicsComponent::PhysicsComponent(ECSManager *ECSManager) {
  // create a dynamic rigidbody

  // btCollisionShape* colShape = new btBoxShape(btVector3(1,1,1));
  colShape = new btSphereShape(btScalar(1.));

  /// Create Dynamic Objects
  btTransform startTransform;
  startTransform.setIdentity();

  btScalar mass(1.f);

  // rigidbody is dynamic if and only if mass is non zero, otherwise static
  bool isDynamic = (mass != 0.f);

  btVector3 localInertia(0, 0, 0);
  if (isDynamic)
    colShape->calculateLocalInertia(mass, localInertia);

  startTransform.setOrigin(btVector3(0, 1, -.1));

  // using motionstate is recommended, it provides interpolation capabilities, and only
  // synchronizes 'active' objects
  btDefaultMotionState *myMotionState = new btDefaultMotionState(startTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
  body = new btRigidBody(rbInfo);

  PhysicsSystem *pSys = static_cast<PhysicsSystem *>(ECSManager->getSystem("PHYSICS"));
  pSys->addRigidBody(body);
}
