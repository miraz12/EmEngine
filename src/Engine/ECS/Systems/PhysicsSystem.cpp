#include "PhysicsSystem.hpp"
#include "GraphicsSystem.hpp"

#include <ECS/Components/DebugComponent.hpp>
#include <ECS/Components/PhysicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Objects/Line.hpp>
#include <Objects/Point.hpp>

PhysicsSystem::~PhysicsSystem() {
  delete m_dynamicsWorld;
  delete m_solver;
  delete m_overlappingPairCache;
  delete m_dispatcher;
  delete m_collisionConfiguration;

  delete m_body;
  delete myMotionState;
  delete groundShape;
}

void PhysicsSystem::initialize(ECSManager &ecsManager) {
  m_manager = &ecsManager;

  /// collision configuration contains default setup for memory, collision
  /// setup. Advanced users can create their own configuration.
  m_collisionConfiguration = new btDefaultCollisionConfiguration();

  /// use the default collision dispatcher. For parallel processing you can use
  /// a diffent dispatcher (see Extras/BulletMultiThreaded)
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

  /// btDbvtBroadphase is a good general purpose broadphase. You can also try
  /// out btAxis3Sweep.
  m_overlappingPairCache = new btDbvtBroadphase();

  /// the default constraint solver. For parallel processing you can use a
  /// different solver (see Extras/BulletMultiThreaded)
  m_solver = new btSequentialImpulseConstraintSolver;

  m_dynamicsWorld = new btDiscreteDynamicsWorld(
      m_dispatcher, m_overlappingPairCache, m_solver, m_collisionConfiguration);

  m_dynamicsWorld->setGravity(btVector3(0, -9.8, 0));
  m_dynamicsWorld->setDebugDrawer(&m_dDraw);

  ///-----initialization_end-----

  // {
  //   groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.),
  //   btScalar(50.)));

  //   btTransform groundTransform;
  //   groundTransform.setIdentity();
  //   groundTransform.setOrigin(btVector3(0, -52, 0));

  //   btScalar mass(0.);

  //   // rigidbody is dynamic if and only if mass is non zero, otherwise static
  //   bool isDynamic = (mass != 0.f);

  //   btVector3 localInertia(0, 0, 0);
  //   if (isDynamic)
  //     groundShape->calculateLocalInertia(mass, localInertia);

  //   // using motionstate is optional, it provides interpolation capabilities,
  //   and only synchronizes
  //   // 'active' objects
  //   myMotionState = new btDefaultMotionState(groundTransform);
  //   btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState,
  //   groundShape, localInertia);

  //   m_body = new btRigidBody(rbInfo);

  //   // add the body to the dynamics world
  //   m_dynamicsWorld->addRigidBody(m_body);
  // }
}

void PhysicsSystem::update(float dt) {

  if (m_manager->getSimulatePhysics()) {
    m_dynamicsWorld->stepSimulation(dt, 10);
    m_dynamicsWorld->debugDrawWorld();
    std::vector<Entity> view =
        m_manager->view<PositionComponent, PhysicsComponent>();
    for (auto e : view) {
      std::shared_ptr<PositionComponent> p =
          m_manager->getComponent<PositionComponent>(e);
      std::shared_ptr<PhysicsComponent> phy =
          m_manager->getComponent<PhysicsComponent>(e);
      if (phy->initialized) {
        btTransform btTrans;
        btRigidBody *body = phy->getRigidBody();
        if (body) {
          body->getMotionState()->getWorldTransform(btTrans);
          btTrans = body->getWorldTransform();
          p->position =
              glm::vec3(btTrans.getOrigin().getX(), btTrans.getOrigin().getY(),
                        btTrans.getOrigin().getZ());
          p->rotation =
              glm::quat(btTrans.getRotation().w(), btTrans.getRotation().x(),
                        btTrans.getRotation().y(), btTrans.getRotation().z());
        }
      } else {
        phy->init();
      }
    }
  }
}
void PhysicsSystem::performPicking(i32 mouseX, i32 mouseY) {

  // Define the ray's start and end positions in world space

  std::tuple<glm::vec3, glm::vec3> camStartDir =
      m_manager->getCamera().getRayTo(mouseX, mouseY);

  glm::vec3 camPos = std::get<0>(camStartDir);
  glm::vec3 rayDir = std::get<1>(camStartDir);
  glm::vec3 endPos = rayDir * 1000.f;
  btVector3 rayTo(endPos.x, endPos.y, endPos.z);
  btVector3 rayFrom(camPos.x, camPos.y, camPos.z);
  btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom, rayTo);
  // Perform raycast
  m_dynamicsWorld->rayTest(rayFrom, rayTo, rayCallback);
  if (rayCallback.hasHit()) {
    // An object was hit by the ray
    // std::shared_ptr<DebugComponent> graphComp =
    // std::make_shared<DebugComponent>(
    //     new Point(rayCallback.m_hitPointWorld.x(),
    //     rayCallback.m_hitPointWorld.y(),
    //               rayCallback.m_hitPointWorld.z()));
    // Entity en = m_manager->createEntity();
    // m_manager->addComponent(en, graphComp);

    btRigidBody *body =
        (btRigidBody *)btRigidBody::upcast(rayCallback.m_collisionObject);
    if (body) {
      m_manager->setEntitySelected(true);
      m_manager->setPickedEntity(body->getUserIndex());
    }
  } else {
    m_manager->setEntitySelected(false);
  }
}
