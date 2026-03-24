#ifndef GEOMETRYPASS_H_
#define GEOMETRYPASS_H_
#include "RenderPasses/RenderPass.hpp"
#include <Graphics/Handle.hpp>

class GeometryPass final : public RenderPass
{
public:
  GeometryPass();
  ~GeometryPass() override = default;
  void Execute(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& fGraph) override;

private:
  gfx::SamplerId m_sampler{};
  gfx::PipelineId m_pipeline{};
  i32 m_modelMatrixLoc{ -1 };
  i32 m_isSkinnedLoc{ -1 };
};

#endif // GEOMETRYPASS_H_
