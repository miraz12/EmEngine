#ifndef DEBUGPASS_H_
#define DEBUGPASS_H_

#include "Managers/FrameBufferManager.hpp"
#include "RenderPasses/RenderPass.hpp"

class DebugPass : public RenderPass {
public:
  DebugPass();
  virtual ~DebugPass() = default;
  void Execute(ECSManager &eManager) override;
};

#endif // DEBUGPASS_H_
