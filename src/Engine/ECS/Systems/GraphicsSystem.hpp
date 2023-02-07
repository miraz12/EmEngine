#ifndef GRAPHICSSYSTEM_H_
#define GRAPHICSSYSTEM_H_

#include <array>

#include "Camera.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/QuadShaderProgram.hpp"
#include "System.hpp"

class GraphicsSystem : public System {
 public:
  GraphicsSystem(ECSManager *ECSManager, Camera &cam);
  void update(float dt);

 private:
  Camera &m_camera;
  FrameBufferManager &m_fboManager;
};
#endif  // GRAPHICSSYSTEM_H_
