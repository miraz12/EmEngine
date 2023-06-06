#ifndef DEBUGPASS_H_
#define DEBUGPASS_H_

#include "Managers/FrameBufferManager.hpp"
#include "RenderPasses/RenderPass.hpp"

class DebugPass : public RenderPass {
public:
  DebugPass();
  virtual ~DebugPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override{};

private:
  uint32_t gBuffer;
  uint32_t rboDepth;
};

#endif // DEBUGPASS_H_
