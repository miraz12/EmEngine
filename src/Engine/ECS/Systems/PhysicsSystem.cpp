#include "PhysicsSystem.hpp"
#include "CameraSystem.hpp"
#include "PhysicsLayers.hpp"

#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/Components/PhysicsComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>

#include <Jolt/Jolt.h>

#include <Jolt/Core/Factory.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyFilter.h>
#include <Jolt/Physics/Body/BodyInterface.h>
#include <Jolt/Physics/Collision/CastResult.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/RayCast.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/TransformedShape.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/RegisterTypes.h>

#ifdef EMSCRIPTEN
#include <Jolt/Core/JobSystemSingleThreaded.h>
#else
#include <Jolt/Core/JobSystemThreadPool.h>
#endif

// Jolt trace/assert callbacks
static void
JoltTraceImpl(const char* inFMT, ...)
{
  va_list list;
  va_start(list, inFMT);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), inFMT, list);
  va_end(list);
  std::cerr << buffer << std::endl;
}

#ifdef JPH_ENABLE_ASSERTS
static bool
JoltAssertFailed(const char* inExpression,
                 const char* inMessage,
                 const char* inFile,
                 unsigned int inLine)
{
  std::cerr << inFile << ":" << inLine << ": (" << inExpression << ") "
            << (inMessage ? inMessage : "") << std::endl;
  return true; // Breakpoint
}
#endif

PhysicsSystem::~PhysicsSystem()
{
  // Remove all bodies from the physics system before destroying it
  if (m_joltSystem) {
    auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();

    // Remove ground body
    if (!m_groundBody.IsInvalid()) {
      bodyInterface.RemoveBody(m_groundBody);
      bodyInterface.DestroyBody(m_groundBody);
    }

    // Remove all tracked entity bodies
    for (auto& [bodyId, entity] : m_bodyToEntity) {
      bodyInterface.RemoveBody(bodyId);
      bodyInterface.DestroyBody(bodyId);
    }
    m_bodyToEntity.clear();
    m_entityToBody.clear();
  }
}

void
PhysicsSystem::initialize(ECSManager& ecsManager)
{
  m_manager = &ecsManager;

  // Install trace/assert callbacks
  JPH::Trace = JoltTraceImpl;
#ifdef JPH_ENABLE_ASSERTS
  JPH::AssertFailed = JoltAssertFailed;
#endif

  // Register allocation hook and physics types
  JPH::RegisterDefaultAllocator();

  if (!JPH::Factory::sInstance) {
    JPH::Factory::sInstance = new JPH::Factory();
  }
  JPH::RegisterTypes();

  // Create temp allocator (10 MB)
  m_tempAllocator = std::make_unique<JPH::TempAllocatorImpl>(10 * 1024 * 1024);

  // Create job system
#ifdef EMSCRIPTEN
  m_jobSystem =
    std::make_unique<JPH::JobSystemSingleThreaded>(JPH::cMaxPhysicsJobs);
#else
  m_jobSystem = std::make_unique<JPH::JobSystemThreadPool>(
    JPH::cMaxPhysicsJobs,
    JPH::cMaxPhysicsBarriers,
    std::thread::hardware_concurrency() - 1);
#endif

  // Create the physics system
  constexpr JPH::uint cMaxBodies = 4096;
  constexpr JPH::uint cNumBodyMutexes = 0; // auto
  constexpr JPH::uint cMaxBodyPairs = 4096;
  constexpr JPH::uint cMaxContactConstraints = 2048;

  static BroadPhaseLayerInterfaceImpl broadPhaseLayerInterface;
  static ObjectVsBroadPhaseLayerFilterImpl objectVsBroadPhaseLayerFilter;
  static ObjectLayerPairFilterImpl objectLayerPairFilter;

  m_joltSystem = std::make_unique<JPH::PhysicsSystem>();
  m_joltSystem->Init(cMaxBodies,
                     cNumBodyMutexes,
                     cMaxBodyPairs,
                     cMaxContactConstraints,
                     broadPhaseLayerInterface,
                     objectVsBroadPhaseLayerFilter,
                     objectLayerPairFilter);

  m_joltSystem->SetGravity(JPH::Vec3(0.0f, -9.8f, 0.0f));

  // Create ground plane (static box at y = -1)
  {
    auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();

    JPH::BoxShapeSettings groundSettings(JPH::Vec3(100.0f, 1.0f, 100.0f));
    auto groundShapeResult = groundSettings.Create();
    JPH::ShapeRefC groundShape = groundShapeResult.Get();

    JPH::BodyCreationSettings groundBodySettings(groundShape,
                                                 JPH::RVec3(0.0f, -1.0f, 0.0f),
                                                 JPH::Quat::sIdentity(),
                                                 JPH::EMotionType::Static,
                                                 Layers::NON_MOVING);

    JPH::Body* groundBody = bodyInterface.CreateBody(groundBodySettings);
    m_groundBody = groundBody->GetID();
    bodyInterface.AddBody(m_groundBody, JPH::EActivation::DontActivate);
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  m_dDraw.initialize();
#endif
}

void
PhysicsSystem::update(float dt)
{
  if (m_manager->getSimulatePhysics()) {
    // Step the physics world
    m_joltSystem->Update(dt, 1, m_tempAllocator.get(), m_jobSystem.get());

    // Sync physics transforms to ECS PositionComponents
    std::vector<Entity> view =
      m_manager->view<PositionComponent, PhysicsComponent>();
    for (auto e : view) {
      auto p = m_manager->getComponent<PositionComponent>(e);
      auto phy = m_manager->getComponent<PhysicsComponent>(e);

      if (!phy || !phy->isValid()) {
        continue;
      }

      auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
      JPH::RVec3 pos = bodyInterface.GetPosition(phy->getBodyID());
      JPH::Quat rot = bodyInterface.GetRotation(phy->getBodyID());

      p->position =
        glm::vec3(float(pos.GetX()), float(pos.GetY()), float(pos.GetZ()));
      p->rotation = glm::quat(rot.GetW(), rot.GetX(), rot.GetY(), rot.GetZ());
    }
  } else {
    // Editor mode: sync selected entity position to physics body
    std::vector<Entity> view =
      m_manager->view<PositionComponent, PhysicsComponent>();
    for (auto e : view) {
      if (e == m_manager->getEntitySelected()) {
        auto p = m_manager->getComponent<PositionComponent>(e);
        auto phy = m_manager->getComponent<PhysicsComponent>(e);

        if (!phy || !phy->isValid()) {
          continue;
        }

        auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
        bodyInterface.SetPositionAndRotation(
          phy->getBodyID(),
          JPH::RVec3(p->position.x, p->position.y, p->position.z),
          JPH::Quat(p->rotation.x, p->rotation.y, p->rotation.z, p->rotation.w),
          JPH::EActivation::DontActivate);
      }
    }
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  // Draw debug wireframe AABBs for all physics bodies
  auto& dbgBodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  std::vector<Entity> debugView =
    m_manager->view<PositionComponent, PhysicsComponent>();
  for (auto e : debugView) {
    auto phy = m_manager->getComponent<PhysicsComponent>(e);
    if (!phy || !phy->isValid()) {
      continue;
    }

    JPH::TransformedShape ts =
      dbgBodyInterface.GetTransformedShape(phy->getBodyID());
    JPH::AABox worldBounds = ts.GetWorldSpaceBounds();
    glm::vec3 mn(worldBounds.mMin.GetX(),
                 worldBounds.mMin.GetY(),
                 worldBounds.mMin.GetZ());
    glm::vec3 mx(worldBounds.mMax.GetX(),
                 worldBounds.mMax.GetY(),
                 worldBounds.mMax.GetZ());

    glm::vec3 color(0.0f, 1.0f, 0.0f);

    // 12 edges of an AABB
    m_dDraw.drawLine({ mn.x, mn.y, mn.z }, { mx.x, mn.y, mn.z }, color);
    m_dDraw.drawLine({ mn.x, mn.y, mn.z }, { mn.x, mx.y, mn.z }, color);
    m_dDraw.drawLine({ mn.x, mn.y, mn.z }, { mn.x, mn.y, mx.z }, color);
    m_dDraw.drawLine({ mx.x, mx.y, mx.z }, { mn.x, mx.y, mx.z }, color);
    m_dDraw.drawLine({ mx.x, mx.y, mx.z }, { mx.x, mn.y, mx.z }, color);
    m_dDraw.drawLine({ mx.x, mx.y, mx.z }, { mx.x, mx.y, mn.z }, color);
    m_dDraw.drawLine({ mn.x, mx.y, mn.z }, { mx.x, mx.y, mn.z }, color);
    m_dDraw.drawLine({ mn.x, mx.y, mn.z }, { mn.x, mx.y, mx.z }, color);
    m_dDraw.drawLine({ mx.x, mn.y, mn.z }, { mx.x, mn.y, mx.z }, color);
    m_dDraw.drawLine({ mx.x, mn.y, mn.z }, { mx.x, mx.y, mn.z }, color);
    m_dDraw.drawLine({ mn.x, mn.y, mx.z }, { mx.x, mn.y, mx.z }, color);
    m_dDraw.drawLine({ mn.x, mn.y, mx.z }, { mn.x, mx.y, mx.z }, color);
  }
#endif
}

JPH::BodyID
PhysicsSystem::createBody(Entity entity, float mass, CollisionShapeType type)
{
  auto posComp =
    ECSManager::getInstance().getComponent<PositionComponent>(entity);
  if (!posComp) {
    return JPH::BodyID();
  }

  glm::vec3 pos = posComp->position;
  glm::vec3 scale = posComp->scale;
  glm::quat rot = posComp->rotation;

  // Create shape based on type
  JPH::ShapeRefC shape;
  switch (type) {
    case CollisionShapeType::BOX: {
      JPH::BoxShapeSettings settings(
        JPH::Vec3(scale.x * 0.5f, scale.y * 0.5f, scale.z * 0.5f));
      auto result = settings.Create();
      if (result.IsValid())
        shape = result.Get();
      break;
    }
    case CollisionShapeType::SPHERE: {
      JPH::SphereShapeSettings settings(scale.x * 0.5f);
      auto result = settings.Create();
      if (result.IsValid())
        shape = result.Get();
      break;
    }
    case CollisionShapeType::CAPSULE: {
      float radius = scale.x * 0.5f;
      float halfHeight = scale.y * 0.25f;
      JPH::CapsuleShapeSettings settings(halfHeight, radius);
      auto result = settings.Create();
      if (result.IsValid())
        shape = result.Get();
      break;
    }
    case CollisionShapeType::CONVEX_HULL:
    case CollisionShapeType::HEIGHTMAP: {
      // Get shape from the entity's GraphicsComponent
      auto graphComp =
        ECSManager::getInstance().getComponent<GraphicsComponent>(entity);
      if (graphComp && graphComp->m_grapObj &&
          graphComp->m_grapObj->p_collisionShape) {
        shape = graphComp->m_grapObj->p_collisionShape;
      }
      // Fallback to a box if no collision shape available
      if (!shape) {
        JPH::BoxShapeSettings settings(JPH::Vec3::sReplicate(0.5f));
        auto result = settings.Create();
        if (result.IsValid())
          shape = result.Get();
      }
      break;
    }
  }

  if (!shape) {
    return JPH::BodyID();
  }

  bool isDynamic = (mass != 0.0f);
  JPH::EMotionType motionType =
    isDynamic ? JPH::EMotionType::Dynamic : JPH::EMotionType::Static;
  JPH::ObjectLayer layer = isDynamic ? Layers::MOVING : Layers::NON_MOVING;

  JPH::BodyCreationSettings bodySettings(shape,
                                         JPH::RVec3(pos.x, pos.y, pos.z),
                                         JPH::Quat(rot.x, rot.y, rot.z, rot.w),
                                         motionType,
                                         layer);

  if (isDynamic) {
    bodySettings.mOverrideMassProperties =
      JPH::EOverrideMassProperties::CalculateInertia;
    bodySettings.mMassPropertiesOverride.mMass = mass;
  }

  // Capsule-specific: lock rotation, low friction, always active
  if (type == CollisionShapeType::CAPSULE && isDynamic) {
    bodySettings.mAllowedDOFs = JPH::EAllowedDOFs::TranslationX |
                                JPH::EAllowedDOFs::TranslationY |
                                JPH::EAllowedDOFs::TranslationZ;
    bodySettings.mFriction = 0.1f;
    bodySettings.mRestitution = 0.0f;
    bodySettings.mLinearDamping = 0.0f;
    bodySettings.mAngularDamping = 0.0f;
  }

  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  JPH::Body* body = bodyInterface.CreateBody(bodySettings);
  if (!body) {
    return JPH::BodyID();
  }

  body->SetUserData(static_cast<u64>(entity));

  JPH::BodyID bodyId = body->GetID();
  bodyInterface.AddBody(bodyId,
                        isDynamic ? JPH::EActivation::Activate
                                  : JPH::EActivation::DontActivate);

  m_bodyToEntity[bodyId] = entity;
  m_entityToBody[entity] = bodyId;

  return bodyId;
}

void
PhysicsSystem::destroyBody(JPH::BodyID bodyId)
{
  if (bodyId.IsInvalid() || !m_joltSystem) {
    return;
  }

  // Only destroy if we still track this body (avoids double-destroy on
  // shutdown)
  auto it = m_bodyToEntity.find(bodyId);
  if (it == m_bodyToEntity.end()) {
    return;
  }

  m_entityToBody.erase(it->second);
  m_bodyToEntity.erase(it);

  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  bodyInterface.RemoveBody(bodyId);
  bodyInterface.DestroyBody(bodyId);
}

void
PhysicsSystem::setLinearVelocity(JPH::BodyID bodyId, float x, float y, float z)
{
  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  if (y == 0.0f) {
    // Preserve existing Y velocity (gravity/jumping)
    JPH::Vec3 vel = bodyInterface.GetLinearVelocity(bodyId);
    bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3(x, vel.GetY(), z));
  } else {
    bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3(x, y, z));
  }
}

void
PhysicsSystem::setHorizontalVelocity(JPH::BodyID bodyId, float x, float z)
{
  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  JPH::Vec3 vel = bodyInterface.GetLinearVelocity(bodyId);
  bodyInterface.SetLinearVelocity(bodyId, JPH::Vec3(x, vel.GetY(), z));
}

void
PhysicsSystem::getLinearVelocity(JPH::BodyID bodyId, float* out)
{
  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  JPH::Vec3 vel = bodyInterface.GetLinearVelocity(bodyId);
  out[0] = vel.GetX();
  out[1] = vel.GetY();
  out[2] = vel.GetZ();
}

void
PhysicsSystem::addImpulse(JPH::BodyID bodyId, float x, float y, float z)
{
  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  bodyInterface.AddImpulse(bodyId, JPH::Vec3(x, y, z));
}

void
PhysicsSystem::addForce(JPH::BodyID bodyId, float x, float y, float z)
{
  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  bodyInterface.AddForce(bodyId, JPH::Vec3(x, y, z));
}

bool
PhysicsSystem::entityOnGround(Entity entity)
{
  auto phy = ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
  if (!phy || !phy->isValid()) {
    return false;
  }

  auto& bodyInterface = m_joltSystem->GetBodyInterfaceNoLock();
  JPH::RVec3 position = bodyInterface.GetPosition(phy->getBodyID());

  // Get actual shape bounds to find the bottom of the body
  JPH::AABox bounds =
    bodyInterface.GetShape(phy->getBodyID())->GetLocalBounds();
  float shapeBottomOffset = bounds.mMin.GetY(); // negative value

  // Start ray from slightly above the shape's bottom
  JPH::RRayCast ray;
  ray.mOrigin = position + JPH::Vec3(0.0f, shapeBottomOffset + 0.05f, 0.0f);
  ray.mDirection = JPH::Vec3(0.0f, -0.3f, 0.0f);

  JPH::RayCastResult result;
  JPH::IgnoreSingleBodyFilter bodyFilter(phy->getBodyID());

  if (m_joltSystem->GetNarrowPhaseQuery().CastRay(
        ray, result, {}, {}, bodyFilter)) {
    return true;
  }
  return false;
}

void
PhysicsSystem::performPicking(i32 mouseX, i32 mouseY)
{
  auto cam = std::static_pointer_cast<CameraComponent>(
    ECSManager::getInstance().getCamera());

  auto [camPos, rayDir] = CameraSystem::getRayTo(cam, mouseX, mouseY);
  glm::vec3 direction = rayDir * 1000.0f - camPos;

  JPH::RRayCast ray;
  ray.mOrigin = JPH::RVec3(camPos.x, camPos.y, camPos.z);
  ray.mDirection = JPH::Vec3(direction.x, direction.y, direction.z);

  JPH::RayCastResult result;

  if (m_joltSystem->GetNarrowPhaseQuery().CastRay(ray, result)) {
    JPH::BodyID hitBodyId = result.mBodyID;
    auto it = m_bodyToEntity.find(hitBodyId);
    if (it != m_bodyToEntity.end() && it->second > 0) {
      m_manager->setEntitySelected(true);
      m_manager->setPickedEntity(it->second);
      return;
    }
  }
  m_manager->setEntitySelected(false);
  m_manager->setPickedEntity(0);
}

void
PhysicsSystem::setWindowSize(float x, float y)
{
  m_winWidth = x;
  m_winHeight = y;
}

void
PhysicsSystem::setGravity(float x, float y, float z)
{
  if (m_joltSystem) {
    m_joltSystem->SetGravity(JPH::Vec3(x, y, z));
  }
}
