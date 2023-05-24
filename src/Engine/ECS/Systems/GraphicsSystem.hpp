#ifndef GRAPHICSSYSTEM_H_
#define GRAPHICSSYSTEM_H_

#include <RenderPasses/FrameGraph.hpp>
#include <array>

#include "Camera.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/QuadShaderProgram.hpp"
#include "System.hpp"

class GraphicsSystem : public System {
public:
  GraphicsSystem(ECSManager *ECSManager);
  void update(float dt);
  void setViewport(unsigned int w, unsigned int h);

private:
  FrameBufferManager &m_fboManager;
  FrameGraph &m_fGraph;
};
#endif // GRAPHICSSYSTEM_H_
