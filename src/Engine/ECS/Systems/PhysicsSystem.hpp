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

class PhysicsSystem : public System {
public:
  PhysicsSystem(ECSManager *ECSManager);
  ~PhysicsSystem();
  void update(float dt);
  void setViewport(unsigned int /* w */, unsigned int /* h */){};
  void addRigidBody(btRigidBody *body) { m_dynamicsWorld->addRigidBody(body); };

private:
  btDiscreteDynamicsWorld *m_dynamicsWorld;
  btDefaultCollisionConfiguration *m_collisionConfiguration;
  btCollisionDispatcher *m_dispatcher;
  btBroadphaseInterface *m_overlappingPairCache;
  btSequentialImpulseConstraintSolver *m_solver;
};
#endif // PHYSICSSYSTEM_H_
