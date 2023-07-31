#ifndef PARTICLEPASS_H_
#define PARTICLEPASS_H_

#include <ECS/ECSManager.hpp>
#include <RenderPasses/RenderPass.hpp>

class ParticlePass : public RenderPass {
public:
  ParticlePass();
  virtual ~ParticlePass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override;

private:
  void renderQuad();
};

#endif // PARTICLEPASS_H_
