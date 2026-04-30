#ifndef RENDERPASS_H_
#define RENDERPASS_H_

#include <Graphics/CommandBuffer.hpp>
#include <Graphics/RenderResources.hpp>
#include <string>
#include <vector>

class ECSManager;
class FrameGraph;

class RenderPass
{
public:
  RenderPass() = delete;
  RenderPass(std::string_view name, std::string_view vs, std::string_view fs);
  virtual ~RenderPass() = default;

  /// Record commands into the command buffer without submitting.
  /// Passes that only need a single record+submit cycle should override this.
  /// Default implementation does nothing (for passes that override Execute).
  virtual void Record(ECSManager& eManager) { (void)eManager; }

  /// Execute the pass. Default implementation calls Record() — passes that
  /// need multiple submits or mid-frame CPU work (e.g. BloomPass) override
  /// this directly.
  virtual void Execute(ECSManager& eManager);

  /// Returns true if this pass manages its own submission (overrides Execute).
  /// Such passes are excluded from batch submission by the FrameGraph.
  [[nodiscard]] virtual bool selfSubmitting() const { return false; }

  /// Get the command buffer handle (for batch submission by FrameGraph)
  [[nodiscard]] gfx::CommandBufferId getCommandBufferId() const
  {
    return m_cmdBuffer;
  }

  virtual void setViewport(u32 /* w */, u32 /* h */) = 0;
  virtual void Init(FrameGraph& /* fGraph */) = 0;
  void addTexture(std::string_view texName);

protected:
  // Shader helpers - use these instead of raw GL calls
  void useShader();
  [[nodiscard]] i32 getUniformLocation(const char* name) const;
  [[nodiscard]] i32 getAttribLocation(const char* name) const;
  [[nodiscard]] gfx::ShaderId getShaderId() const;

  static constexpr u32 kDefaultWidth = 800;
  static constexpr u32 kDefaultHeight = 800;

  u32 m_width{ kDefaultWidth };
  u32 m_height{ kDefaultHeight };
  std::string m_shaderName;
  std::vector<std::string> m_textures;

  // Track if resources were created through the new RenderResources system
  // This enables gradual migration - passes can check this flag in
  // setViewport() to determine whether to use
  // RenderResources::recreateTexture2D() or legacy code
  bool m_useNewResources{ false };

  // Command buffer for deferred rendering
  gfx::CommandBufferId m_cmdBuffer;

  // Get command buffer pointer (creates if needed, resets for new frame)
  [[nodiscard]] gfx::CommandBuffer* getCommandBuffer();
};

#endif // RENDERPASS_H_
