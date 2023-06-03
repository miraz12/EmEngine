#include "PhysicsSystem.hpp"
#include <ECS/Components/PhysicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>

PhysicsSystem::~PhysicsSystem() {
  delete m_dynamicsWorld;
  delete m_solver;
  delete m_overlappingPairCache;
  delete m_dispatcher;
  delete m_collisionConfiguration;

  delete body;
  delete myMotionState;
  delete groundShape;
}

void PhysicsSystem::initialize(ECSManager &ecsManager) {
  m_manager = &ecsManager;

  /// collision configuration contains default setup for memory, collision setup. Advanced users can
  /// create their own configuration.
  m_collisionConfiguration = new btDefaultCollisionConfiguration();

  /// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher
  /// (see Extras/BulletMultiThreaded)
  m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);

  /// btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
  m_overlappingPairCache = new btDbvtBroadphase();

  /// the default constraint solver. For parallel processing you can use a different solver (see
  /// Extras/BulletMultiThreaded)
  m_solver = new btSequentialImpulseConstraintSolver;

  m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher, m_overlappingPairCache, m_solver,
                                                m_collisionConfiguration);

  m_dynamicsWorld->setGravity(btVector3(0, -0.01, 0));

  ///-----initialization_end-----

  {
    groundShape = new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

    btTransform groundTransform;
    groundTransform.setIdentity();
    groundTransform.setOrigin(btVector3(0, -52, 0));

    btScalar mass(0.);

    // rigidbody is dynamic if and only if mass is non zero, otherwise static
    bool isDynamic = (mass != 0.f);

    btVector3 localInertia(0, 0, 0);
    if (isDynamic)
      groundShape->calculateLocalInertia(mass, localInertia);

    // using motionstate is optional, it provides interpolation capabilities, and only synchronizes
    // 'active' objects
    myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    body = new btRigidBody(rbInfo);

    // add the body to the dynamics world
    m_dynamicsWorld->addRigidBody(body);
  }
}

void PhysicsSystem::update(float dt) {

  m_dynamicsWorld->stepSimulation(dt, 10);
  std::vector<Entity> view = m_manager->view<PositionComponent, PhysicsComponent>();
  for (auto e : view) {
    std::shared_ptr<PositionComponent> p = m_manager->getComponent<PositionComponent>(e);
    std::shared_ptr<PhysicsComponent> phy = m_manager->getComponent<PhysicsComponent>(e);
    btTransform btTrans;
    if (body) {
      body->getMotionState()->getWorldTransform(btTrans);
      btTrans = body->getWorldTransform();
      p->position = glm::vec3(btTrans.getOrigin().getX(), btTrans.getOrigin().getY(),
                              btTrans.getOrigin().getZ());
      printf("world pos object = %f,%f,%f\n", float(btTrans.getOrigin().getX()),
             float(btTrans.getOrigin().getY()), float(btTrans.getOrigin().getZ()));
    }
  }
}
