#ifndef LIGHTPASS_H_
#define LIGHTPASS_H_
#include "Managers/FrameBufferManager.hpp"
#include "RenderPasses/RenderPass.hpp"
#include <Camera.hpp>
#include <Managers/TextureManager.hpp>

class LightPass : public RenderPass {
public:
  LightPass();
  virtual ~LightPass(){};
  void Execute(ECSManager &eManager) override;
  void setViewport(u32 w, u32 h) override;

private:
  u32 quadVAO;
  u32 m_lightBuffer, m_rbo;
};

#endif // LIGHTPASS_H_
