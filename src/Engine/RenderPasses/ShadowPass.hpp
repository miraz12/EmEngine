#ifndef SHADOWPASS_H_
#define SHADOWPASS_H_

#include "RenderPasses/RenderPass.hpp"

class ShadowPass final : public RenderPass
{
public:
  ShadowPass();
  ~ShadowPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& fGraph) override;

private:
  bool m_dirty{ false };
};

#endif // SHADOWPASS_H_
