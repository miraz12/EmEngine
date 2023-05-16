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
  void setViewport(unsigned int w, unsigned int h) override;

private:
  unsigned int quadVAO;
};

#endif // LIGHTPASS_H_
