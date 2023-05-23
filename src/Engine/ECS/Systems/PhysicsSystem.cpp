#include "PhysicsSystem.hpp"
#include <btBulletDynamicsCommon.h>

PhysicsSystem::PhysicsSystem(ECSManager *ECSManager, Camera &cam) : System(ECSManager) {
  /// collision configuration contains default setup for memory, collision setup. Advanced users can
  /// create their own configuration.
  btDefaultCollisionConfiguration *collisionConfiguration = new btDefaultCollisionConfiguration();

  /// use the default collision dispatcher. For parallel processing you can use a diffent dispatcher
  /// (see Extras/BulletMultiThreaded)
  btCollisionDispatcher *dispatcher = new btCollisionDispatcher(collisionConfiguration);

  /// btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
  btBroadphaseInterface *overlappingPairCache = new btDbvtBroadphase();

  /// the default constraint solver. For parallel processing you can use a different solver (see
  /// Extras/BulletMultiThreaded)
  btSequentialImpulseConstraintSolver *solver = new btSequentialImpulseConstraintSolver;

  m_dynamicsWorld =
      new btDiscreteDynamicsWorld(dispatcher, overlappingPairCache, solver, collisionConfiguration);

  m_dynamicsWorld->setGravity(btVector3(0, -10, 0));

  ///-----initialization_end-----

  // keep track of the shapes, we release memory at exit.
  // make sure to re-use collision shapes among rigid bodies whenever possible!
  btAlignedObjectArray<btCollisionShape *> collisionShapes;
}

void PhysicsSystem::update(float dt) { m_dynamicsWorld->stepSimulation(dt); }