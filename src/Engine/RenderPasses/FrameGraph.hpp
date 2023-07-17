#ifndef FRAMEGRAPH_H_
#define FRAMEGRAPH_H_

#include "Camera.hpp"
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/QuadShaderProgram.hpp>
#include <array>

// Moving a pass enum to the right of kNumPasses will dactivate it.
enum class PassId : size_t { kShadow, kGeom, kLight, kCube, kBloom, kNumPasses, kDebug };

class FrameGraph : public Singleton<FrameGraph> {
  friend class Singleton<FrameGraph>;

public:
  FrameGraph();
  ~FrameGraph();

  void draw(ECSManager &eManager);
  void setViewport(uint32_t w, uint32_t h);

  std::array<RenderPass *, static_cast<size_t>(PassId::kNumPasses)> m_renderPass;

private:
  uint32_t m_width{800}, m_height{800};
  uint32_t m_quadVAO = 0;
  uint32_t m_quadVBO = 0;
};

#endif // FRAMEGRAPH_H_
