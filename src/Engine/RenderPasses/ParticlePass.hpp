#ifndef PARTICLEPASS_H_
#define PARTICLEPASS_H_

#include <ECS/ECSManager.hpp>
#include <Graphics/Handle.hpp>
#include <RenderPasses/RenderPass.hpp>

class ParticlePass final : public RenderPass
{
public:
  ParticlePass();
  ~ParticlePass() override = default;
  void Record(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  // Pipeline for particle rendering
  gfx::PipelineId m_pipeline{};

  // Cached uniform locations for performance
  i32 m_projMatrixLoc{ -1 };
  i32 m_viewMatrixLoc{ -1 };
  i32 m_particlePosLoc{ -1 };
  i32 m_colorLoc{ -1 };
};

#endif // PARTICLEPASS_H_
