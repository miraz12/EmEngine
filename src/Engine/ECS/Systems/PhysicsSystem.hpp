#ifndef PHYSICSSYSTEM_H_
#define PHYSICSSYSTEM_H_

#include "ECS/Components/PhysicsComponent.hpp"
#include "System.hpp"
#include <ECS/ECSManager.hpp>
#include <Managers/FrameBufferManager.hpp>
#include <Rendering/DebugDrawer.hpp>
#include <ShaderPrograms/QuadShaderProgram.hpp>

class PhysicsSystem
  : public System
  , public Singleton<PhysicsSystem>
{
  friend class Singleton<PhysicsSystem>;

public:
  void initialize(ECSManager& ecsManager) override;
  void update(float dt) override;
  void setViewport(u32 /* w */, u32 /* h */){};
  // Add rigid body to physics sim
  void addRigidBody(btRigidBody* body) { m_dynamicsWorld->addRigidBody(body); };
  // Function to perform raycasting and pick an object
  void performPicking(i32 mouseX, i32 mouseY);
  void setWindowSize(float x, float y);
  void CreatePhysicsBody(Entity entity, PhysicsComponent& physicsComponent);
  bool EntityOnGround(Entity entity);

  // Debug drawing options
  DebugDrawer m_dDraw;
  bool m_debugDrawEnabled = true; // Toggle for debug drawing

  // Toggle debug drawing on/off
  void toggleDebugDrawing() { m_debugDrawEnabled = !m_debugDrawEnabled; }

private:
  PhysicsSystem() = default;
  ~PhysicsSystem() override;
  btDiscreteDynamicsWorld* m_dynamicsWorld;
  btDefaultCollisionConfiguration* m_collisionConfiguration;
  btCollisionDispatcher* m_dispatcher;
  btBroadphaseInterface* m_overlappingPairCache;
  btSequentialImpulseConstraintSolver* m_solver;
  btRigidBody* m_body;
  btDefaultMotionState* myMotionState;
  btCollisionShape* groundShape;
  float m_winWidth, m_winHeigth;
};
#endif // PHYSICSSYSTEM_H_
