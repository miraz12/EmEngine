#ifndef SHADOWPASS_H_
#define SHADOWPASS_H_

#include "RenderPasses/LightingUtil.hpp"
#include "RenderPasses/RenderPass.hpp"

class ShadowPass final : public RenderPass
{
public:
  ShadowPass();
  ~ShadowPass() override = default;
  void Record(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& fGraph) override;

private:
  static constexpr u32 NUM_CASCADES = LightingUtil::CascadeConfig::NUM_CASCADES;
  static constexpr u32 SHADOW_MAP_SIZE =
    LightingUtil::CascadeConfig::SHADOW_MAP_SIZE;

  // Note: cascadeUBO is now managed by RenderResources
  LightingUtil::CascadeConfig m_cascadeConfig;

  // Cached uniform locations for CommandBuffer use
  i32 m_lightSpaceMatrixLoc{ -1 };
  i32 m_modelMatrixLoc{ -1 };
  i32 m_isSkinnedLoc{ -1 };

  // Pipeline for CommandBuffer rendering
  gfx::PipelineId m_pipeline{};
};

#endif // SHADOWPASS_H_
