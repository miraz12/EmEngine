#include "PhysicsComponent.hpp"

#include "DebugComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include <BulletDynamics/Character/btKinematicCharacterController.h>
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <Objects/Cube.hpp>

using enum CollisionShapeType;

PhysicsComponent::~PhysicsComponent()
{
  delete shape;
  delete body;
  delete myMotionState;
};

PhysicsComponent::PhysicsComponent(Entity en,
                                   float mass,
                                   CollisionShapeType type)
  : m_en(en)
  , mass(mass)
{

  std::shared_ptr<PositionComponent> posComp =
    ECSManager::getInstance().getComponent<PositionComponent>(m_en);

  if (posComp) {
    initialPos =
      btVector3(posComp->position.x, posComp->position.y, posComp->position.z);
    // HACK: Changed rotation axels to work with plane..
    initialScale =
      btVector3(posComp->scale.x, posComp->scale.z, posComp->scale.y);
    initialRotation = btQuaternion(posComp->rotation.x,
                                   posComp->rotation.y,
                                   posComp->rotation.z,
                                   posComp->rotation.w);

    switch (type) {
      case BOX:
        shape =
          new btBoxShape(btVector3(btScalar(1.), btScalar(1.), btScalar(1.)));
        break;
      case SPHERE:
        shape = new btSphereShape(btScalar(1.));
        break;
      case CAPSULE:
        // new btKinematicCharacterController();
        shape = new btCapsuleShape(btScalar(1.), btScalar(1.));
        break;
      case CONVEX_HULL:
      case HEIGHTMAP:
        setupCollisionShapeFromGra();
        break;
    }

    ECSManager& eManager = ECSManager::getInstance();

    /// Create Dynamic Objects
    startTransform.setIdentity();

    // rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
      shape->calculateLocalInertia(mass, localInertia);

    startTransform.setOrigin(initialPos);
    startTransform.setRotation(initialRotation);

    // using motionstate is recommended, it provides interpolation capabilities,
    // and only synchronizes 'active' objects
    myMotionState = new btDefaultMotionState(startTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(
      mass, myMotionState, shape, localInertia);
    body = new btRigidBody(rbInfo);
    if (type == CAPSULE) {
      // Configure rigid body for character movement
      body->setAngularFactor(btVector3(0, 0, 0)); // No rotation
      body->setFriction(0.1f);      // Very low friction for faster running
      body->setRestitution(0.0f);   // No bouncing
      body->setDamping(0.0f, 0.0f); // No damping for maximum speed
      body->setActivationState(DISABLE_DEACTIVATION);
      body->setCollisionFlags(btCollisionObject::CF_CHARACTER_OBJECT);
    }

    body->setUserIndex(m_en);

    auto& pSys = dynamic_cast<PhysicsSystem&>(eManager.getSystem("PHYSICS"));
    pSys.addRigidBody(body);
  }
}

void
PhysicsComponent::setupCollisionShapeFromGra()
{
  std::shared_ptr<GraphicsComponent> graphComp =
    ECSManager::getInstance().getComponent<GraphicsComponent>(m_en);
  if (graphComp) {
    shape = graphComp->m_grapObj->p_coll;
    shape->setLocalScaling(initialScale);
  }
}
