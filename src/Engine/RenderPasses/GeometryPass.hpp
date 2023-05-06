#ifndef GEOMETRYPASS_H_
#define GEOMETRYPASS_H_
#include "Camera.hpp"
#include "Managers/FrameBufferManager.hpp"
#include "RenderPasses/RenderPass.hpp"

class GeometryPass : public RenderPass {
public:
  GeometryPass();
  virtual ~GeometryPass() = default;
  void Execute(ECSManager &eManager) override;
  void setViewport(unsigned int w, unsigned int h) override;

private:
  unsigned int quadVAO;
  unsigned int gBuffer;
  unsigned int gPosition, gNormal, gAlbedo, gEmissive;
  unsigned int rboDepth;
};

#endif // GEOMETRYPASS_H_
