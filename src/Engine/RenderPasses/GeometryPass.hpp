#ifndef GEOMETRYPASS_H_
#define GEOMETRYPASS_H_
#include "Managers/FrameBufferManager.hpp"
#include "RenderPasses/RenderPass.hpp"

class GeometryPass : public RenderPass {
public:
  GeometryPass();
  virtual ~GeometryPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(uint32_t w, uint32_t h) override;

private:
  uint32_t gBuffer;
  uint32_t rboDepth;
};

#endif // GEOMETRYPASS_H_
