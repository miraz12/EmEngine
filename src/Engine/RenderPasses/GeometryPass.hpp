#ifndef GEOMETRYPASS_H_
#define GEOMETRYPASS_H_
#include "RenderPasses/RenderPass.hpp"
#include <Graphics/Handle.hpp>

class GeometryPass final : public RenderPass
{
public:
  GeometryPass();
  ~GeometryPass() override = default;
  void Record(ECSManager& eManager) override;
  void setViewport(u32 w, u32 h) override;
  void Init(FrameGraph& fGraph) override;

private:
  gfx::SamplerId m_sampler{};
  gfx::PipelineId m_pipeline{};
  i32 m_isSkinnedLoc{ -1 };

  // Instanced rendering (batched non-skinned entities)
  gfx::BufferId m_instanceBuffer{};
  u32 m_instanceBufferCapacity{ 0 };
  static constexpr u32 kInitialInstanceCapacity = 256;

  // Single-instance buffer for skinned entity draws (separate to avoid
  // overwriting batched data before command buffer execution)
  gfx::BufferId m_singleInstanceBuffer{};
};

#endif // GEOMETRYPASS_H_
