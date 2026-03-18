#ifndef PARTICLEPASS_H_
#define PARTICLEPASS_H_

#include <ECS/ECSManager.hpp>
#include <RenderPasses/RenderPass.hpp>

class ParticlePass final : public RenderPass
{
public:
  ParticlePass();
  ~ParticlePass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  // Cached uniform locations for performance
  GLint m_projMatrixLoc{ -1 };
  GLint m_viewMatrixLoc{ -1 };
  GLint m_particlePosLoc{ -1 };
  GLint m_colorLoc{ -1 };
};

#endif // PARTICLEPASS_H_
