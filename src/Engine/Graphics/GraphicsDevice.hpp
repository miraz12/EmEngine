#pragma once

#include "CommandBuffer.hpp"
#include "GraphicsTypes.hpp"
#include "Handle.hpp"
#include "Resources/Buffer.hpp"
#include "Resources/Framebuffer.hpp"
#include "Resources/Pipeline.hpp"
#include "Resources/Renderbuffer.hpp"
#include "Resources/Sampler.hpp"
#include "Resources/Shader.hpp"
#include "Resources/Texture.hpp"
#include "Singleton.hpp"
#include <memory>
#include <optional>
#include <span>

namespace gfx {

namespace gles3 {
class Device;
}

/// Backend-agnostic GPU resource management singleton.
class GraphicsDevice : public Singleton<GraphicsDevice>
{
  friend class Singleton<GraphicsDevice>;

public:
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

  /// Attach a texture to a framebuffer attachment point.
  /// For cubemaps: layer specifies face (0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z)
  /// @param fbo Framebuffer to modify
  /// @param attachmentIndex Color attachment index (0-7)
  /// @param texture Texture to attach
  /// @param mipLevel Mip level to attach (default 0)
  /// @param layer Array layer or cubemap face (default 0)
  void setFramebufferAttachment(FramebufferId fbo,
                                u32 attachmentIndex,
                                TextureId texture,
                                u32 mipLevel = 0,
                                u32 layer = 0);

  /// Attach a depth texture to a framebuffer.
  /// For texture arrays: layer specifies array slice
  /// For cubemaps: layer specifies face (0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z)
  /// @param fbo Framebuffer to modify
  /// @param texture Depth texture to attach
  /// @param mipLevel Mip level to attach (default 0)
  /// @param layer Array layer or cubemap face (default 0)
  void setFramebufferDepthAttachment(FramebufferId fbo,
                                     TextureId texture,
                                     u32 mipLevel = 0,
                                     u32 layer = 0);

  /// Attach a renderbuffer to a framebuffer.
  /// @param fbo Framebuffer to modify
  /// @param attachmentType Depth, Stencil, or DepthStencil
  /// @param rbo Renderbuffer to attach
  void setFramebufferRenderbuffer(FramebufferId fbo,
                                  RenderbufferAttachment attachmentType,
                                  RenderbufferId rbo);

  /// Set which color attachments will be drawn to.
  /// @param fbo Framebuffer to modify
  /// @param attachments Span of color attachment indices (0-7). Empty =
  /// depth-only.
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

  void submit(CommandBufferId cmdBuffer);
  void submit(std::span<const CommandBufferId> cmdBuffers);

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

  // Native handle access (for interop during migration)
  [[nodiscard]] u32 getNativeHandle(BufferId buffer) const;
  [[nodiscard]] u32 getNativeHandle(TextureId texture) const;
  [[nodiscard]] u32 getNativeHandle(ShaderId program) const;
  [[nodiscard]] u32 getNativeHandle(FramebufferId framebuffer) const;
  [[nodiscard]] u32 getNativeHandle(RenderbufferId renderbuffer) const;

  [[nodiscard]] i32 getUniformLocation(ShaderId program,
                                       const char* name) const;
  [[nodiscard]] i32 getAttribLocation(ShaderId program, const char* name) const;

  // Uniform setting (for gradual shader migration)
  void setUniformInt(i32 location, i32 value);
  void setUniformIntArray(i32 location, std::span<const i32> values);
  void setUniformFloat(i32 location, float value);
  void setUniformVec2(i32 location, const glm::vec2& value);
  void setUniformVec3(i32 location, const glm::vec3& value);
  void setUniformVec4(i32 location, const glm::vec4& value);
  void setUniformMat3(i32 location, const glm::mat3& value);
  void setUniformMat4(i32 location, const glm::mat4& value);

  // Default framebuffer (screen)
  [[nodiscard]] FramebufferId getDefaultFramebuffer() const;

  // Render state management

  /// Pass invalid handle or getDefaultFramebuffer() for the screen framebuffer.
  void bindFramebuffer(FramebufferId fbo);

  void setViewport(i32 x, i32 y, u32 width, u32 height);

  /// Requires enableScissorTest() to be active.
  void setScissor(i32 x, i32 y, u32 width, u32 height);

  /// Clear color attachment(s). Must have framebuffer bound.
  /// @param attachment Color attachment index (0-7), or nullopt to clear all
  /// @param color RGBA clear color
  void clearColor(std::optional<u32> attachment, const glm::vec4& color);

  /// Clear depth buffer. Must have framebuffer bound.
  void clearDepth(float depth = 1.0f);

  /// Clear stencil buffer. Must have framebuffer bound.
  void clearStencil(i32 value = 0);

  /// Clear depth and stencil buffers together. Must have framebuffer bound.
  void clearDepthStencil(float depth = 1.0f, i32 stencil = 0);

  void setDepthTest(bool enable);

  /// Set depth comparison function (requires depth test enabled).
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

  /// Bind a texture to a texture unit.
  /// @param unit Texture unit (0-15)
  /// @param texture Texture to bind
  void bindTexture(u32 unit, TextureId texture);

  /// Bind a sampler to a texture unit.
  /// @param unit Texture unit (0-15)
  /// @param sampler Sampler to bind (invalid handle unbinds)
  void bindSampler(u32 unit, SamplerId sampler);

  /// Copy pixels between framebuffers (blit).
  /// @param srcFbo Source framebuffer
  /// @param dstFbo Destination framebuffer
  /// @param srcX0, srcY0, srcX1, srcY1 Source rectangle
  /// @param dstX0, dstY0, dstX1, dstY1 Destination rectangle
  /// @param colorBit Copy color attachment
  /// @param depthBit Copy depth attachment
  /// @param stencilBit Copy stencil attachment
  /// @param linearFilter Use linear filtering (only for color)
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
                       bool linearFilter = false);

  /// Enable or disable seamless cubemap filtering (desktop GL only).
  /// When enabled, texture fetches at cubemap edges interpolate across faces.
  void setSeamlessCubemap(bool enable);

  void setLineWidth(float width);

  /// Enable or disable line smoothing (desktop GL only).
  void setLineSmoothing(bool enable);

  void setColorMask(bool red, bool green, bool blue, bool alpha);

  /// Set blend constant color (for ConstantColor/ConstantAlpha blend factors).
  void setBlendColor(float red, float green, float blue, float alpha);

  /// Set read buffer for the currently bound framebuffer.
  /// @param attachment Color attachment index (0-7), or nullopt for GL_NONE
  void setReadBuffer(std::optional<u32> attachment);

  /// Bind a uniform buffer to a binding point.
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
  GraphicsDevice();
  ~GraphicsDevice();

  // Prevent move/copy (unique_ptr with incomplete type requires explicit dtor)
  GraphicsDevice(const GraphicsDevice&) = delete;
  GraphicsDevice& operator=(const GraphicsDevice&) = delete;

  std::unique_ptr<gles3::Device> m_backend;
};

} // namespace gfx
