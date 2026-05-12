#ifndef FRAMEGRAPH_H_
#define FRAMEGRAPH_H_

#include <RenderPasses/RenderPass.hpp>
#include <array>

#ifndef NDEBUG
class Profiler;
#endif

// Moving a pass enum to the right of kNumPasses will deactivate it.
enum class PassId : size_t
{
  kShadow,
  kGeom,
  kLight,
  kCube,
  kParticle,
  kBloom,
  kFxaa,
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  kDebug,
#endif
  kNumPasses
};

class FrameGraph : public Singleton<FrameGraph>
{
  friend class Singleton<FrameGraph>;

public:
  FrameGraph();
  ~FrameGraph() = default;

  void draw(ECSManager& eManager);
  void setViewport(u32 w, u32 h);

  RenderPass* getPass(PassId id)
  {
    return m_renderPass[static_cast<size_t>(id)].get();
  }
  const RenderPass* getPass(PassId id) const
  {
    return m_renderPass[static_cast<size_t>(id)].get();
  }

private:
  std::array<std::unique_ptr<RenderPass>,
             static_cast<size_t>(PassId::kNumPasses)>
    m_renderPass;
  u32 m_width{ 800 };
  u32 m_height{ 800 };

#ifndef NDEBUG
  Profiler* m_profiler{ nullptr };

public:
  void setProfiler(Profiler* prof) { m_profiler = prof; }
#endif
};

#endif // FRAMEGRAPH_H_
