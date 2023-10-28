#ifndef FXAAPASS_H_
#define FXAAPASS_H_

#include <ECS/ECSManager.hpp>
#include <RenderPasses/RenderPass.hpp>

class FxaaPass : public RenderPass {
public:
  FxaaPass();
  virtual ~FxaaPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(u32 w, u32 h) override;
};

#endif // FXAAPASS_H_
