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
  void setViewport(uint32_t w, uint32_t h) override;

private:
  uint32_t quadVAO;
  uint32_t m_lightBuffer, m_rbo;
};

#endif // LIGHTPASS_H_
