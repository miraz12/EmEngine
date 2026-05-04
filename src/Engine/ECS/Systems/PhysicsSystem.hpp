#ifndef PHYSICSSYSTEM_H_
#define PHYSICSSYSTEM_H_

#include "ECS/Components/PhysicsComponent.hpp"
#include "System.hpp"
#include <Rendering/DebugDrawer.hpp>

#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Jolt.h>
#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Physics/PhysicsSystem.h>

#ifdef EMSCRIPTEN
#include <Jolt/Core/JobSystemSingleThreaded.h>
#else
#include <Jolt/Core/JobSystemThreadPool.h>
#endif

class ECSManager;

class PhysicsSystem
  : public System
  , public Singleton<PhysicsSystem>
{
  friend class Singleton<PhysicsSystem>;

public:
  void initialize(ECSManager& ecsManager) override;
  void update(float dt) override;
  void setViewport(u32, u32){};

  // Body lifecycle
  JPH::BodyID createBody(Entity entity, float mass, CollisionShapeType type);
  void destroyBody(JPH::BodyID bodyId);

  // Physics operations
  void setLinearVelocity(JPH::BodyID bodyId, float x, float y, float z);
  void setHorizontalVelocity(JPH::BodyID bodyId, float x, float z);
  void getLinearVelocity(JPH::BodyID bodyId, float* out);
  void addImpulse(JPH::BodyID bodyId, float x, float y, float z);
  void addForce(JPH::BodyID bodyId, float x, float y, float z);

  // Queries
  bool entityOnGround(Entity entity);
  void performPicking(i32 mouseX, i32 mouseY);
  void setWindowSize(float x, float y);

  // Configuration
  void setGravity(float x, float y, float z);

  DebugDrawer& getDebugDrawer() { return m_dDraw; }

private:
  PhysicsSystem() = default;
  ~PhysicsSystem() override;

  std::unique_ptr<JPH::PhysicsSystem> m_joltSystem;
  std::unique_ptr<JPH::TempAllocatorImpl> m_tempAllocator;
  std::unique_ptr<JPH::JobSystem> m_jobSystem;

  // Entity <-> BodyID mapping
  std::unordered_map<JPH::BodyID, Entity> m_bodyToEntity;
  std::unordered_map<Entity, JPH::BodyID> m_entityToBody;

  JPH::BodyID m_groundBody;

  float m_winWidth{ 0 };
  float m_winHeight{ 0 };
  DebugDrawer m_dDraw;
};

#endif // PHYSICSSYSTEM_H_
