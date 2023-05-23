#include "PhysicsSystem.hpp"
#include <ECS/Components/PhysicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>

PhysicsSystem::PhysicsSystem(ECSManager *ECSManager, Camera &cam) : System(ECSManager) {
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

  m_dynamicsWorld->setGravity(btVector3(0, -1, 0));

  ///-----initialization_end-----

  {
    btCollisionShape *groundShape =
        new btBoxShape(btVector3(btScalar(50.), btScalar(50.), btScalar(50.)));

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
    btDefaultMotionState *myMotionState = new btDefaultMotionState(groundTransform);
    btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, groundShape, localInertia);
    btRigidBody *body = new btRigidBody(rbInfo);

    // add the body to the dynamics world
    m_dynamicsWorld->addRigidBody(body);
  }
}

PhysicsSystem::~PhysicsSystem() {
  // delete dynamics world
  delete m_dynamicsWorld;
  // delete solver
  delete m_solver;
  // delete broadphase
  delete m_overlappingPairCache;
  // delete dispatcher
  delete m_dispatcher;
  delete m_collisionConfiguration;
}

void PhysicsSystem::update(float dt) {

  m_dynamicsWorld->stepSimulation(dt, 10);
  std::vector<Entity> view = m_manager->view<PositionComponent, PhysicsComponent>();
  for (auto e : view) {
    PositionComponent *p = m_manager->getComponent<PositionComponent>(e);
    PhysicsComponent *phy = m_manager->getComponent<PhysicsComponent>(e);
    btRigidBody *body = phy->getRigidBody();
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
