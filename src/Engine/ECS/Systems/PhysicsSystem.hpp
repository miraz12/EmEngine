#ifndef PHYSICSSYSTEM_H_
#define PHYSICSSYSTEM_H_

#include <RenderPasses/FrameGraph.hpp>
#include <array>

#include "Camera.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/QuadShaderProgram.hpp"
#include "System.hpp"
#include <btBulletDynamicsCommon.h>

class PhysicsSystem : public System {
public:
  PhysicsSystem(ECSManager *ECSManager, Camera &cam);
  void update(float dt);
  void setViewport(unsigned int w, unsigned int h){};

private:

  btDiscreteDynamicsWorld *m_dynamicsWorld;
};
#endif // PHYSICSSYSTEM_H_
