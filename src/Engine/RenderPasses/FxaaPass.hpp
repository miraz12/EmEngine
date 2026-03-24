#ifndef FXAAPASS_H_
#define FXAAPASS_H_

#include <RenderPasses/RenderPass.hpp>

/// FXAA (Fast Approximate Anti-Aliasing) post-processing pass.
///
/// Input: "frameBloomFinal" texture from BloomPass
/// Output: Default framebuffer (screen)
class FxaaPass final : public RenderPass
{
public:
  FxaaPass();
  ~FxaaPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  gfx::PipelineId m_pipeline;
};

#endif // FXAAPASS_H_
