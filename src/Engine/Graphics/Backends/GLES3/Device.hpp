#pragma once

#include "../../CommandBuffer.hpp"
#include "Resources.hpp"
#include <array>
#include <optional>
#include <span>

namespace gfx::gles3 {

class Device
{
public:
  Device() = default;
  ~Device();

  Device(const Device&) = delete;
  Device& operator=(const Device&) = delete;

  bool initialize();

  void shutdown();

  BufferId createBuffer(const BufferCreateInfo& info);
  void destroyBuffer(BufferId buffer);

  TextureId createTexture(const TextureCreateInfo& info);
  void destroyTexture(TextureId texture);

  SamplerId createSampler(const SamplerCreateInfo& info);
  void destroySampler(SamplerId sampler);

  ShaderId createShader(const ShaderCreateInfo& info);
  ShaderId createShaderProgram(const ShaderProgramCreateInfo& info);
  void destroyShader(ShaderId shader);

  PipelineId createPipeline(const PipelineCreateInfo& info);
  void destroyPipeline(PipelineId pipeline);

  FramebufferId createFramebuffer(const FramebufferCreateInfo& info);
  void destroyFramebuffer(FramebufferId framebuffer);
  void setFramebufferAttachment(FramebufferId fbo,
                                u32 attachmentIndex,
                                TextureId texture,
                                u32 mipLevel,
                                u32 layer);
  void setFramebufferDepthAttachment(FramebufferId fbo,
                                     TextureId texture,
                                     u32 mipLevel,
                                     u32 layer);
  void setFramebufferRenderbuffer(FramebufferId fbo,
                                  RenderbufferAttachment attachmentType,
                                  RenderbufferId rbo);
  void setDrawBuffers(FramebufferId fbo, std::span<const u32> attachments);

  RenderbufferId createRenderbuffer(const RenderbufferCreateInfo& info);
  void destroyRenderbuffer(RenderbufferId renderbuffer);
  void resizeRenderbuffer(RenderbufferId renderbuffer, u32 width, u32 height);

  VertexArrayId createVertexArray(const VertexArrayCreateInfo& info);
  void destroyVertexArray(VertexArrayId vao);
  void bindVertexArray(VertexArrayId vao);
  void drawVertexArray(VertexArrayId vao, BufferId vertexBuffer);
  void drawVertexArrayDynamic(VertexArrayId vao,
                              BufferId vertexBuffer,
                              PrimitiveTopology topology,
                              u32 vertexCount);
  void drawIndexedVertexArray(VertexArrayId vao,
                              BufferId vertexBuffer,
                              BufferId indexBuffer,
                              u32 indexCount,
                              IndexType indexType,
                              u32 offset = 0);

  void updateBuffer(BufferId buffer, u64 offset, const void* data, u64 size);
  void* mapBuffer(BufferId buffer);
  void unmapBuffer(BufferId buffer);

  // Texture operations
  void updateTexture(TextureId texture,
                     u32 mipLevel,
                     u32 layer,
                     const void* data,
                     u64 dataSize);
  void resizeTexture(TextureId texture,
                     u32 newWidth,
                     u32 newHeight,
                     const void* data = nullptr);
  void generateMipmaps(TextureId texture);

  CommandBufferId createCommandBuffer();
  void destroyCommandBuffer(CommandBufferId cmdBuffer);
  CommandBuffer* getCommandBuffer(CommandBufferId cmdId);

  void executeCommandBuffer(const CommandBuffer& cmdBuffer);

  void beginFrame();
  void endFrame();

  [[nodiscard]] bool isValid(BufferId handle) const;
  [[nodiscard]] bool isValid(TextureId handle) const;
  [[nodiscard]] bool isValid(SamplerId handle) const;
  [[nodiscard]] bool isValid(ShaderId handle) const;
  [[nodiscard]] bool isValid(PipelineId handle) const;
  [[nodiscard]] bool isValid(FramebufferId handle) const;
  [[nodiscard]] bool isValid(RenderbufferId handle) const;
  [[nodiscard]] bool isValid(VertexArrayId handle) const;

  [[nodiscard]] bool isFramebufferComplete(FramebufferId fbo) const;

  // Native handle access (for interop)
  [[nodiscard]] GLuint getNativeHandle(BufferId buffer) const;
  [[nodiscard]] GLuint getNativeHandle(TextureId texture) const;
  [[nodiscard]] GLuint getNativeHandle(ShaderId program) const;
  [[nodiscard]] GLuint getNativeHandle(FramebufferId framebuffer) const;
  [[nodiscard]] GLuint getNativeHandle(RenderbufferId renderbuffer) const;

  [[nodiscard]] GLint getUniformLocation(ShaderId program,
                                         const char* name) const;
  [[nodiscard]] GLint getAttribLocation(ShaderId program,
                                        const char* name) const;

  // Uniform setting (for gradual shader migration)
  void setUniformInt(i32 location, i32 value);
  void setUniformIntArray(i32 location, std::span<const i32> values);
  void setUniformFloat(i32 location, float value);
  void setUniformVec2(i32 location, const glm::vec2& value);
  void setUniformVec3(i32 location, const glm::vec3& value);
  void setUniformVec4(i32 location, const glm::vec4& value);
  void setUniformMat3(i32 location, const glm::mat3& value);
  void setUniformMat4(i32 location, const glm::mat4& value);

  [[nodiscard]] FramebufferId getDefaultFramebuffer() const;

  // Render state management (immediate mode, for migration from raw GL)

  void bindFramebuffer(FramebufferId fbo);

  void setViewport(i32 x, i32 y, u32 width, u32 height);

  void setScissor(i32 x, i32 y, u32 width, u32 height);

  /// Clear color attachment(s). nullopt = clear all.
  void clearColor(std::optional<u32> attachment, const glm::vec4& color);

  void clearDepth(float depth);

  void clearStencil(i32 value);

  void clearDepthStencil(float depth, i32 stencil);

  void setDepthTest(bool enable);

  void setDepthFunc(CompareOp op);

  void setDepthWrite(bool enable);

  void setCullMode(CullMode mode);

  void setFrontFace(FrontFace face);

  void setBlendEnabled(bool enable);

  void setBlendFunc(BlendFactor srcColor,
                    BlendFactor dstColor,
                    BlendFactor srcAlpha,
                    BlendFactor dstAlpha);

  void setBlendEquation(BlendOp colorOp, BlendOp alphaOp);

  void setScissorTest(bool enable);

  void bindShaderProgram(ShaderId program);

  void bindTexture(u32 unit, TextureId texture);

  void bindSampler(u32 unit, SamplerId sampler);

  void blitFramebuffer(FramebufferId srcFbo,
                       FramebufferId dstFbo,
                       i32 srcX0,
                       i32 srcY0,
                       i32 srcX1,
                       i32 srcY1,
                       i32 dstX0,
                       i32 dstY0,
                       i32 dstX1,
                       i32 dstY1,
                       bool colorBit,
                       bool depthBit,
                       bool stencilBit,
                       bool linearFilter);

  void setSeamlessCubemap(bool enable);

  void setLineWidth(float width);

  void setLineSmoothing(bool enable);

  void setColorMask(bool r, bool g, bool b, bool a);

  void setBlendColor(float r, float g, float b, float a);

  /// Set read buffer for the currently bound framebuffer
  /// attachment: 0-7 for color attachments, nullopt for GL_NONE
  void setReadBuffer(std::optional<u32> attachment);

  /// Bind a uniform buffer to a binding point
  /// @param bindingPoint UBO binding point (0-15)
  /// @param buffer Uniform buffer to bind
  /// @param offset Offset in bytes from the start of the buffer
  /// @param size Size in bytes of the buffer range to bind
  void bindUniformBuffer(u32 bindingPoint,
                         BufferId buffer,
                         u64 offset,
                         u64 size);

  /// Bind a shader's uniform block to a binding point.
  /// This must be called once per shader program to associate the block name
  /// with a binding point.
  /// @param program Shader program
  /// @param blockName Name of the uniform block in the shader
  /// @param bindingPoint UBO binding point (0-15)
  void bindUniformBlock(ShaderId program,
                        const char* blockName,
                        u32 bindingPoint);

private:
  void processDeletionQueues();

  // GL state cache to minimize redundant calls
  struct StateCache
  {
    GLuint boundProgram{ 0 };
    GLuint boundVAO{ 0 };
    GLuint boundFBO{ 0 };
    std::array<GLuint, 16> boundTextures{};
    std::array<GLenum, 16> boundTextureTargets{};
    std::array<GLuint, 16> boundSamplers{};
    PipelineId currentPipeline;
    VertexArrayId explicitVAO{}; // VAO bound via bindVertexArray command

    void reset()
    {
      boundProgram = 0;
      boundVAO = 0;
      boundFBO = 0;
      boundTextures.fill(0);
      boundTextureTargets.fill(0);
      boundSamplers.fill(0);
      currentPipeline = {};
      explicitVAO = {};
    }
  };

  void executeBeginRenderPass(const RenderPassBeginInfo& info);
  void executeEndRenderPass();
  void executeBindPipeline(PipelineId pipeline);
  void executeSetViewport(const Viewport& viewport);
  void executeSetScissor(const ScissorRect& scissor);
  void executeBindVertexBuffer(u32 binding, BufferId buffer, u64 offset);
  void executeBindIndexBuffer(BufferId buffer, u64 offset, IndexType type);
  void executeBindTexture(u32 slot, TextureId texture, SamplerId sampler);
  void executeDraw(u32 vertexCount,
                   u32 instanceCount,
                   u32 firstVertex,
                   u32 firstInstance);
  void executeDrawIndexed(u32 indexCount,
                          u32 instanceCount,
                          u32 firstIndex,
                          i32 vertexOffset,
                          u32 firstInstance);

  static GLenum toGLInternalFormat(PixelFormat format);
  static GLenum toGLFormat(PixelFormat format);
  static GLenum toGLType(PixelFormat format);
  static GLenum toGLTarget(TextureType type);
  static GLenum toGLBufferUsage(BufferUsage usage);
  static GLenum toGLPrimitive(PrimitiveTopology topology);
  static GLenum toGLCompareFunc(CompareOp op);
  static GLenum toGLBlendFactor(BlendFactor factor);
  static GLenum toGLBlendOp(BlendOp op);
  static GLenum toGLFilter(FilterMode mode);
  static GLenum toGLMagFilter(
    FilterMode mode); // Mag filter can't use mipmap modes
  static GLenum toGLWrap(WrapMode mode);
  static GLenum toGLIndexType(IndexType type);

  // Vertex attribute format conversion
  struct VertexAttribInfo
  {
    GLint size;
    GLenum type;
    GLboolean normalized;
    bool
      isInteger; // Use glVertexAttribIPointer instead of glVertexAttribPointer
  };
  static VertexAttribInfo toGLVertexAttrib(PixelFormat format);

  ResourcePool<Buffer, BufferId> m_buffers;
  ResourcePool<Texture, TextureId> m_textures;
  ResourcePool<Sampler, SamplerId> m_samplers;
  ResourcePool<ShaderProgram, ShaderId> m_shaders;
  ResourcePool<Pipeline, PipelineId> m_pipelines;
  ResourcePool<Framebuffer, FramebufferId> m_framebuffers;
  ResourcePool<Renderbuffer, RenderbufferId> m_renderbuffers;
  ResourcePool<VertexArray, VertexArrayId> m_vertexArrays;
  ResourcePool<CommandBuffer, CommandBufferId> m_commandBuffers;

  StateCache m_stateCache;
  FramebufferId m_defaultFramebuffer;

  // Deferred deletion (to avoid deleting in-flight resources)
  static constexpr u32 kDeletionDelay = 3;
  u32 m_frameIndex{ 0 };
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingBufferDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingTextureDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingSamplerDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingShaderDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingProgramDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingFramebufferDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingRenderbufferDeletes;
  std::array<std::vector<GLuint>, kDeletionDelay> m_pendingVAODeletes;

  // Bound index buffer state for draw calls
  GLenum m_boundIndexType{ GL_UNSIGNED_SHORT };
  u64 m_boundIndexBufferOffset{ 0 };

  bool m_initialized{ false };
};

} // namespace gfx::gles3
