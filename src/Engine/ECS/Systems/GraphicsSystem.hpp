#ifndef GRAPHICSSYSTEM_H_
#define GRAPHICSSYSTEM_H_

#include <RenderPasses/FrameGraph.hpp>
#include <array>

#include "Camera.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "ShaderPrograms/QuadShaderProgram.hpp"
#include "System.hpp"

class GraphicsSystem : public System, public Singleton<GraphicsSystem> {
  friend class Singleton<GraphicsSystem>;

public:
  void update(float dt) override;
  void setViewport(uint32_t w, uint32_t h);

private:
  GraphicsSystem();
  ~GraphicsSystem();
  FrameBufferManager &m_fboManager;
  FrameGraph &m_fGraph;
};
#endif // GRAPHICSSYSTEM_H_
