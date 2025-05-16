#ifndef FRAMEGRAPH_H_
#define FRAMEGRAPH_H_

#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/QuadShaderProgram.hpp>
#include <array>

// Moving a pass enum to the right of kNumPasses will dactivate it.
enum class PassId : size_t
{
  kShadow,
  kGeom,
  kLight,
  kCube,
  kParticle,
  kBloom,
  kFxaa,
  kDebug,
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

  std::array<std::unique_ptr<RenderPass>,
             static_cast<size_t>(PassId::kNumPasses)>
    m_renderPass;

private:
  u32 m_width{ 800 };
  u32 m_height{ 800 };
};

#endif // FRAMEGRAPH_H_
