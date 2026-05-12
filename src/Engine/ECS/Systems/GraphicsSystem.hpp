#ifndef GRAPHICSSYSTEM_H_
#define GRAPHICSSYSTEM_H_

#include "System.hpp"

#include <RenderPasses/FrameGraph.hpp>

class GraphicsSystem final
  : public System
  , public Singleton<GraphicsSystem>
{
  friend class Singleton<GraphicsSystem>;

public:
  void update(float dt) override;
  void setViewport(u32 w, u32 h);
#ifndef NDEBUG
  void setProfiler(Profiler* prof) { m_fGraph->setProfiler(prof); }
#endif

private:
  GraphicsSystem();
  ~GraphicsSystem() override;
  std::unique_ptr<FrameGraph> m_fGraph;
};
#endif // GRAPHICSSYSTEM_H_
