#pragma once

#include "GraphicsTypes.hpp"
#include "Handle.hpp"
#include "UBOStructs.hpp"
#include <array>
#include <glm/glm.hpp>
#include <string>
#include <vector>

namespace gfx {

/// Viewport specification
struct Viewport
{
  float x{ 0 };
  float y{ 0 };
  float width{ 0 };
  float height{ 0 };
  float minDepth{ 0.0f };
  float maxDepth{ 1.0f };
};

/// Scissor rectangle
struct ScissorRect
{
  i32 x{ 0 };
  i32 y{ 0 };
  u32 width{ 0 };
  u32 height{ 0 };
};

/// Render pass begin parameters
struct RenderPassBeginInfo
{
  static constexpr u32 kMaxColorAttachments = 8;

  FramebufferId framebuffer;
  std::array<glm::vec4, kMaxColorAttachments> clearColors{};
  float clearDepth{ 1.0f };
  u8 clearStencil{ 0 };
  u8 colorAttachmentCount{ 1 };
  bool clearColor{ true };
  bool clearDepthStencil{ true };
};

/// Command types for the command stream
enum class CommandType : u8
{
  // Render pass
  BeginRenderPass,
  EndRenderPass,
  // Pipeline & state
  BindPipeline,
  SetViewport,
  SetScissor,
  // Resource binding
  BindVertexArray, // VAO binding for per-entity rendering
  BindVertexBuffer,
  BindIndexBuffer,
  BindUniformBuffer,
  BindTexture,
  BindTextureByName, // Texture lookup by string name at execution time
                     // Framebuffer modification
  SetFramebufferAttachment,
  SetFramebufferDepthAttachment,
  // Clearing
  ClearDepth,
  // Render state
  SetCullMode,
  SetBlendEnabled,
  SetBlendFunc,
  FlushMaterialUBO,  // Flush MaterialUBO from RenderResources (deprecated)
  UpdateMaterialUBO, // Update MaterialUBO with inline data (for command buffer)
                     // Uniforms (legacy support)
  SetUniformMat4,
  SetUniformVec4,
  SetUniformVec3,
  SetUniformVec2,
  SetUniformFloat,
  SetUniformInt,
  // Draw calls
  Draw,
  DrawIndexed,
  // Debug
  PushDebugGroup,
  PopDebugGroup,
};

/// Records rendering commands for deferred execution.
/// Commands are encoded into a byte stream and executed by the backend.
class CommandBuffer
{
public:
  CommandBuffer() = default;
  ~CommandBuffer() = default;

  // Non-copyable, movable
  CommandBuffer(const CommandBuffer&) = delete;
  CommandBuffer& operator=(const CommandBuffer&) = delete;
  CommandBuffer(CommandBuffer&&) = default;
  CommandBuffer& operator=(CommandBuffer&&) = default;

  void beginRenderPass(const RenderPassBeginInfo& info);
  void endRenderPass();

  void bindPipeline(PipelineId pipeline);

  void setViewport(const Viewport& viewport);
  void setScissor(const ScissorRect& scissor);

  /// Bind vertex array object (VAO) for per-entity rendering
  void bindVertexArray(VertexArrayId vao);
  void bindVertexBuffer(u32 binding, BufferId buffer, u64 offset = 0);
  void bindIndexBuffer(BufferId buffer, u64 offset, IndexType indexType);
  void bindUniformBuffer(u32 binding, BufferId buffer, u64 offset, u64 size);
  void bindTexture(u32 slot, TextureId texture, SamplerId sampler);
  /// Bind texture by name (lookup deferred to execution time via
  /// RenderResources)
  void bindTextureByName(u32 slot,
                         const std::string& textureName,
                         SamplerId sampler);

  // Framebuffer modification (for dynamic attachment changes)
  void setFramebufferAttachment(FramebufferId fbo,
                                u32 attachmentIndex,
                                TextureId texture,
                                u32 mipLevel = 0,
                                u32 layer = 0);
  void setFramebufferDepthAttachment(FramebufferId fbo,
                                     TextureId texture,
                                     u32 mipLevel = 0,
                                     u32 layer = 0);

  void clearDepth(float depth = 1.0f);

  // Render state commands (for material binding)
  void setCullMode(CullMode mode);
  void setBlendEnabled(bool enabled);
  void setBlendFunc(BlendFactor srcColor,
                    BlendFactor dstColor,
                    BlendFactor srcAlpha,
                    BlendFactor dstAlpha);
  /// Flush MaterialUBO from RenderResources (must call after updating
  /// MaterialUBO data) DEPRECATED: Use updateMaterialUBO() instead for proper
  /// deferred rendering
  void flushMaterialUBO();
  /// Update MaterialUBO with the provided data (stored in command stream)
  void updateMaterialUBO(const MaterialUBO& data);

  // Legacy uniform setting (for gradual migration)
  void setUniform(i32 location, const glm::mat4& value);
  void setUniform(i32 location, const glm::vec4& value);
  void setUniform(i32 location, const glm::vec3& value);
  void setUniform(i32 location, const glm::vec2& value);
  void setUniform(i32 location, float value);
  void setUniform(i32 location, i32 value);

  void draw(u32 vertexCount,
            u32 instanceCount = 1,
            u32 firstVertex = 0,
            u32 firstInstance = 0);
  void drawIndexed(u32 indexCount,
                   u32 instanceCount = 1,
                   u32 firstIndex = 0,
                   i32 vertexOffset = 0,
                   u32 firstInstance = 0);

  void pushDebugGroup(const char* name);
  void popDebugGroup();

  [[nodiscard]] bool isEmpty() const { return m_commandStream.empty(); }
  [[nodiscard]] size_t size() const { return m_commandStream.size(); }

  void reset();

  // Access to raw command stream (for backend execution)
  [[nodiscard]] const std::vector<u8>& getCommandStream() const
  {
    return m_commandStream;
  }
  [[nodiscard]] const std::vector<std::string>& getDebugNames() const
  {
    return m_debugNames;
  }

private:
  template<typename T>
  void encode(const T& value);

  void encodeCommand(CommandType cmd);

  std::vector<u8> m_commandStream;
  std::vector<std::string> m_debugNames;
};

} // namespace gfx
