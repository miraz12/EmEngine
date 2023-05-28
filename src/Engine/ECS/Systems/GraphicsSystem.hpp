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
  GraphicsSystem(ECSManager &ECSManager);
  ~GraphicsSystem();
  void update(float dt);
  void setViewport(uint32_t w, uint32_t h);

private:
  FrameBufferManager &m_fboManager;
  FrameGraph &m_fGraph;
};
#endif // GRAPHICSSYSTEM_H_
