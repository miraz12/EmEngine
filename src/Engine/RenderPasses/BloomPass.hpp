#ifndef BLOOMPASS_H_
#define BLOOMPASS_H_

#include "RenderPasses/RenderPass.hpp"
#include <Graphics/RenderResources.hpp>

class BloomPass final : public RenderPass
{
public:
  BloomPass();
  ~BloomPass() override = default;
  void Execute(ECSManager& eManager) override;
  [[nodiscard]] bool selfSubmitting() const override { return true; }
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  struct mipLevel
  {
    glm::vec2 size;
    glm::ivec2 intSize;
    std::string textureName; // Name registered with RenderResources
  };

  std::vector<mipLevel> m_mipChain;

  // Shader names (loaded via RenderResources)
  std::string m_extractBrightName{ "ExtractBright" };
  std::string m_downShaderName{ "BloomDown" };
  std::string m_bloomCombineName{ "BloomCombine" };

  // Cached uniform locations for sampler bindings (cannot be in UBO)
  // Note: resolution, exposure, filterRadius, mipLevel now come from
  // PostProcessData UBO
  GLint m_combineSceneLoc{ -1 };
  GLint m_combineBloomBlurLoc{ -1 };

  // Pipelines for CommandBuffer rendering
  gfx::PipelineId m_extractBrightPipeline;
  gfx::PipelineId m_downPipeline;
  gfx::PipelineId m_upPipeline;
  gfx::PipelineId m_combinePipeline;
};

#endif // BLOOMPASS_H_
