#ifndef FXAAPASS_H_
#define FXAAPASS_H_

#include <ECS/ECSManager.hpp>
#include <RenderPasses/RenderPass.hpp>

class FxaaPass : public RenderPass {
public:
  FxaaPass();
  virtual ~FxaaPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override;

private:
  void renderQuad();
};

#endif // FXAAPASS_H_
