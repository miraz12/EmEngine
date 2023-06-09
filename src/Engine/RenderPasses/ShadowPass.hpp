#ifndef SHADOWPASS_H_
#define SHADOWPASS_H_

#include "RenderPasses/RenderPass.hpp"
#include "glm/fwd.hpp"

class ShadowPass : public RenderPass {
public:
  ShadowPass();
  virtual ~ShadowPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override;
  void Init(FrameGraph &fGraph) override;

private:
  bool m_dirty{false};
};

#endif // SHADOWPASS_H_
