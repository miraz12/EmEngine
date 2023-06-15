#include "PhysicsComponent.hpp"
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <ECS/Components/DebugComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/PhysicsSystem.hpp>
#include <LinearMath/btDefaultMotionState.h>
#include <Objects/Cube.hpp>

PhysicsComponent::PhysicsComponent() { init(nullptr); }

PhysicsComponent::~PhysicsComponent() {
  delete colShape;
  delete body;
  delete myMotionState;
};

PhysicsComponent::PhysicsComponent(std::shared_ptr<PositionComponent> posComp, uint32_t type,
                                   float mass, std::shared_ptr<GraphicsComponent> graphComp)
    : mass(mass), m_type(type) {
  if (posComp) {
    initialPos = btVector3(posComp->position.x, posComp->position.y, posComp->position.z);
    initialScale = btVector3(posComp->scale.x, posComp->scale.y, posComp->scale.z);
  }
  init(graphComp);
}

void PhysicsComponent::init(std::shared_ptr<GraphicsComponent> /* graphComp */) {
  // create a dynamic rigidbody

  switch (m_type) {
  case 0:
    colShape = new btBoxShape(btVector3(0.5, 0.5, 0.5) * initialScale);
    break;
  case 1:
    colShape = new btSphereShape(btScalar(0.5 * initialScale.x()));
    break;
  case 2:
    // btBvhTriangleMeshShape *shape = new btBvhTriangleMeshShape(graphComp->m_grapObj.p_mesh,
    // true); colShape = shape; Triangle mesh
    break;
  }

  ECSManager &eManager = ECSManager::getInstance();
  std::shared_ptr<DebugComponent> dComp = std::make_shared<DebugComponent>(new Cube());
  // HACK gets the current id that this component will be added to.. fix this
  eManager.addComponent(eManager.getLastEntity(), dComp);

  /// Create Dynamic Objects
  startTransform.setIdentity();

  // rigidbody is dynamic if and only if mass is non zero, otherwise static
  bool isDynamic = (mass != 0.f);

  btVector3 localInertia(0, 0, 0);
  if (isDynamic)
    colShape->calculateLocalInertia(mass, localInertia);

  startTransform.setOrigin(initialPos);

  // using motionstate is recommended, it provides interpolation capabilities, and only
  // synchronizes 'active' objects
  myMotionState = new btDefaultMotionState(startTransform);
  btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);
  body = new btRigidBody(rbInfo);
  body->setUserIndex(eManager.getLastEntity());

  PhysicsSystem &pSys = static_cast<PhysicsSystem &>(eManager.getSystem("PHYSICS"));
  pSys.addRigidBody(body);
}
