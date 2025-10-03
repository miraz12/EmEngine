#ifndef DEBUGPASS_H_
#define DEBUGPASS_H_
#ifndef EMSCRIPTEN
#include "RenderPasses/RenderPass.hpp"

class DebugPass final : public RenderPass
{
public:
  DebugPass();
  ~DebugPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 /* w */, u32 /* h */) override{};
  void Init(FrameGraph& /* fGraph */) override {};
};

#endif
#endif // DEBUGPASS_H_
