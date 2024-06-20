#ifndef GEOMETRYPASS_H_
#define GEOMETRYPASS_H_
#include "RenderPasses/RenderPass.hpp"

class GeometryPass final : public RenderPass {
public:
  GeometryPass();
  ~GeometryPass() override = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph &fGraph) override;

private:
  u32 gBuffer;
  u32 rboDepth;
};

#endif // GEOMETRYPASS_H_
