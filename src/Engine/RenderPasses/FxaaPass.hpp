#ifndef FXAAPASS_H_
#define FXAAPASS_H_

#include <ECS/ECSManager.hpp>
#include <RenderPasses/RenderPass.hpp>

class FxaaPass final : public RenderPass
{
public:
  FxaaPass();
  ~FxaaPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  // Cached uniform locations for performance
  GLint m_sceneLoc{ -1 };
  GLint m_resolutionLoc{ -1 };
};

#endif // FXAAPASS_H_
