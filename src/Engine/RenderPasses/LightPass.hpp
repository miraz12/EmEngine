#ifndef LIGHTPASS_H_
#define LIGHTPASS_H_
#include "RenderPasses/RenderPass.hpp"
#include <Managers/TextureManager.hpp>

class LightPass final : public RenderPass
{
public:
  LightPass();
  ~LightPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& /* fGraph */) override {};

private:
  u32 quadVAO;
  u32 m_lightBuffer;
  u32 m_rbo;
};

#endif // LIGHTPASS_H_
