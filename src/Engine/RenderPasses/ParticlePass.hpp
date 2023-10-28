#ifndef PARTICLEPASS_H_
#define PARTICLEPASS_H_

#include <ECS/ECSManager.hpp>
#include <RenderPasses/RenderPass.hpp>

class ParticlePass : public RenderPass {
public:
  ParticlePass();
  virtual ~ParticlePass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(u32 w, u32 h) override;
};

#endif // PARTICLEPASS_H_
