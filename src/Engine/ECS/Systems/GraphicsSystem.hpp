#ifndef GRAPHICSSYSTEM_H_
#define GRAPHICSSYSTEM_H_

#include "System.hpp"

#include <Camera.hpp>
#include <Managers/FrameBufferManager.hpp>
#include <RenderPasses/FrameGraph.hpp>

class GraphicsSystem : public System, public Singleton<GraphicsSystem> {
  friend class Singleton<GraphicsSystem>;

public:
  void update(float dt) override;
  virtual void setViewport(u32 w, u32 h);

private:
  GraphicsSystem();
  ~GraphicsSystem();
  FrameBufferManager &m_fboManager;
  FrameGraph &m_fGraph;
};
#endif // GRAPHICSSYSTEM_H_
