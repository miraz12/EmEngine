#ifndef LIGHTPASS_H_
#define LIGHTPASS_H_
#include "RenderPasses/RenderPass.hpp"

class LightPass final : public RenderPass
{
public:
  LightPass();
  ~LightPass() override = default;
  void Record(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  static constexpr u32 MAX_POINT_LIGHTS = 10;
  // Note: All light uniforms now come from LightingData UBO
  // Camera data comes from CameraData UBO

  gfx::PipelineId m_pipeline;
};

#endif // LIGHTPASS_H_
