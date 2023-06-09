#ifndef FRAMEGRAPH_H_
#define FRAMEGRAPH_H_

#include "Camera.hpp"
#include <RenderPasses/RenderPass.hpp>
#include <ShaderPrograms/QuadShaderProgram.hpp>
#include <array>

enum class PassId : size_t {
  kShadow,
  kGeom,
  kLight,
  kCube,
#ifdef _DEBUG_
  kDebug,
#endif
  kNumPasses
};

class FrameGraph {
public:
  FrameGraph();
  ~FrameGraph() = default;

  void draw(ECSManager &eManager);
  void setViewport(uint32_t w, uint32_t h);

  std::array<RenderPass *, static_cast<size_t>(PassId::kNumPasses)> m_renderPass;

private:
  uint32_t m_width{800}, m_height{800};
  QuadShaderProgram m_quadShader;
};

#endif // FRAMEGRAPH_H_
