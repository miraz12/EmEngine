#ifndef PHYSICSSYSTEM_H_
#define PHYSICSSYSTEM_H_

#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <RenderPasses/FrameGraph.hpp>
#include <array>

#include "Camera.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/QuadShaderProgram.hpp"
#include "System.hpp"
#include <btBulletDynamicsCommon.h>

class PhysicsSystem : public System, public Singleton<PhysicsSystem> {
  friend class Singleton<PhysicsSystem>;

public:
  void initialize(ECSManager &ecsManager) override;
  void update(float dt) override;
  void setViewport(uint32_t /* w */, uint32_t /* h */){};
  void addRigidBody(btRigidBody *body) { m_dynamicsWorld->addRigidBody(body); };

private:
  PhysicsSystem() = default;
  ~PhysicsSystem() override;
  btDiscreteDynamicsWorld *m_dynamicsWorld;
  btDefaultCollisionConfiguration *m_collisionConfiguration;
  btCollisionDispatcher *m_dispatcher;
  btBroadphaseInterface *m_overlappingPairCache;
  btSequentialImpulseConstraintSolver *m_solver;
  btRigidBody *m_body;
  btDefaultMotionState *myMotionState;
  btCollisionShape *groundShape;
};
#endif // PHYSICSSYSTEM_H_
