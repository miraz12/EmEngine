#ifndef SHADOWPASS_H_
#define SHADOWPASS_H_

#include "RenderPasses/LightingUtil.hpp"
#include "RenderPasses/RenderPass.hpp"

class ShadowPass final : public RenderPass
{
public:
  ShadowPass();
  ~ShadowPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& fGraph) override;

private:
  static constexpr u32 NUM_CASCADES = LightingUtil::CascadeConfig::NUM_CASCADES;
  static constexpr u32 SHADOW_MAP_SIZE =
    LightingUtil::CascadeConfig::SHADOW_MAP_SIZE;

  u32 m_cascadeUBO{0};
  LightingUtil::CascadeConfig m_cascadeConfig;
};

#endif // SHADOWPASS_H_
