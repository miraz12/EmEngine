#ifndef SHADOWPASS_H_
#define SHADOWPASS_H_

#include "RenderPasses/RenderPass.hpp"

class ShadowPass : public RenderPass {
public:
  ShadowPass();
  virtual ~ShadowPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(unsigned int w, unsigned int h) override;

private:
  unsigned int m_depthMapFBO;
  unsigned int m_depthMap;
};

#endif // SHADOWPASS_H_
