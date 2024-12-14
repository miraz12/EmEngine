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

bool PhysicsSystem::EntityOnGround(Entity entity) {
  std::shared_ptr<PhysicsComponent> phyComp =
      ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
  if (!phyComp->body)
    return false;

  // Get the bottom center of the object
  btTransform transform = phyComp->body->getWorldTransform();
  btVector3 start = transform.getOrigin();

  // Get collision shape's height to cast from bottom
  btVector3 extent =
      static_cast<btBoxShape *>(phyComp->body->getCollisionShape())
          ->getHalfExtentsWithoutMargin();
  start.setY(start.y() - extent.y()); // Move to bottom of shape

  btVector3 end = start + btVector3(0, -1.06, 0);

  // Perform raycast
  btCollisionWorld::ClosestRayResultCallback rayCallback(start, end);
  m_dynamicsWorld->rayTest(start, end, rayCallback);

  return rayCallback.hasHit();
}

void PhysicsSystem::CreatePhysicsBody(Entity entity,
                                      PhysicsComponent &physicsComponent) {
  btCollisionShape *shape = nullptr;

  switch (physicsComponent.shapeType) {
  case CollisionShapeType::BOX:
    shape = new btBoxShape(physicsComponent.dimensions *
                           0.5f); // Box dimensions are half-extents
    break;

  case CollisionShapeType::SPHERE:
    shape = new btSphereShape(
        physicsComponent.dimensions.x()); // Use x as the radius
    break;

  case CollisionShapeType::CAPSULE:
    shape = new btCapsuleShape(physicsComponent.capsuleRadius,
                               physicsComponent.capsuleHeight);
    break;

  case CollisionShapeType::CONVEX_HULL:
    // Assuming you already have a convex mesh defined in the component
    shape = new btConvexTriangleMeshShape(physicsComponent.mesh);
    break;

  default:
    // Handle error or unsupported shape
    std::cerr << "Error: Unsupported collision shape type!" << std::endl;
    return;
  }

  // Calculate inertia for dynamic bodies (mass > 0)
  btVector3 localInertia(0, 0, 0);
  if (physicsComponent.mass > 0.0f) {
    shape->calculateLocalInertia(physicsComponent.mass, localInertia);
  }

  // Set up motion state and initial transform
  btTransform startTransform;
  startTransform.setIdentity();
  startTransform.setOrigin(
      physicsComponent.dimensions); // Set initial position (if applicable)

  btDefaultMotionState *motionState = new btDefaultMotionState(startTransform);

  btRigidBody::btRigidBodyConstructionInfo rbInfo(
      physicsComponent.mass, motionState, shape, localInertia);
  physicsComponent.body = new btRigidBody(rbInfo);

  // Optionally disable rotation (common for characters)
  if (physicsComponent.shapeType == CollisionShapeType::CAPSULE) {
    physicsComponent.body->setAngularFactor(btVector3(0, 0, 0));
  }

  // Add the body to the dynamics world
  m_dynamicsWorld->addRigidBody(physicsComponent.body);

  // Store the shape in the component for later cleanup
  physicsComponent.shape = shape;
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
    }
  } else {
    std::vector<Entity> view =
        m_manager->view<PositionComponent, PhysicsComponent>();
    for (auto e : view) {
      if (e == m_manager->getEntitySelected()) {
        std::shared_ptr<PositionComponent> p =
            m_manager->getComponent<PositionComponent>(e);
        std::shared_ptr<PhysicsComponent> phy =
            m_manager->getComponent<PhysicsComponent>(e);

        // Update bullet
        btTransform btTrans;
        btTrans.setFromOpenGLMatrix(glm::value_ptr(p->model));

        btRigidBody *body = phy->getRigidBody();
        body->setWorldTransform(btTrans);

        // Recalculate aabbs
        btCollisionWorld *collisionWorld = m_dynamicsWorld->getCollisionWorld();
        collisionWorld->updateAabbs();
        collisionWorld->computeOverlappingPairs();
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
    m_manager->setPickedEntity(0);
  }
}
