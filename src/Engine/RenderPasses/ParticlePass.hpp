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
};

#endif // PARTICLEPASS_H_
