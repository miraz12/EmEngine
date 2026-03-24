#include "GraphicsDevice.hpp"
#include "Backends/GLES3/Device.hpp"

namespace gfx {

// Constructor/destructor must be in .cpp where Device is complete
GraphicsDevice::GraphicsDevice() = default;

GraphicsDevice::~GraphicsDevice()
{
  shutdown();
}

bool
GraphicsDevice::initialize()
{
  if (m_backend) {
    return true;
  }

  m_backend = std::make_unique<gles3::Device>();
  return m_backend->initialize();
}

void
GraphicsDevice::shutdown()
{
  if (m_backend) {
    m_backend->shutdown();
    m_backend.reset();
  }
}

BufferId
GraphicsDevice::createBuffer(const BufferCreateInfo& info)
{
  return m_backend ? m_backend->createBuffer(info) : BufferId{};
}

void
GraphicsDevice::destroyBuffer(BufferId buffer)
{
  if (m_backend) {
    m_backend->destroyBuffer(buffer);
  }
}

TextureId
GraphicsDevice::createTexture(const TextureCreateInfo& info)
{
  return m_backend ? m_backend->createTexture(info) : TextureId{};
}

void
GraphicsDevice::destroyTexture(TextureId texture)
{
  if (m_backend) {
    m_backend->destroyTexture(texture);
  }
}

SamplerId
GraphicsDevice::createSampler(const SamplerCreateInfo& info)
{
  return m_backend ? m_backend->createSampler(info) : SamplerId{};
}

void
GraphicsDevice::destroySampler(SamplerId sampler)
{
  if (m_backend) {
    m_backend->destroySampler(sampler);
  }
}

ShaderId
GraphicsDevice::createShader(const ShaderCreateInfo& info)
{
  return m_backend ? m_backend->createShader(info) : ShaderId{};
}

ShaderId
GraphicsDevice::createShaderProgram(const ShaderProgramCreateInfo& info)
{
  return m_backend ? m_backend->createShaderProgram(info) : ShaderId{};
}

void
GraphicsDevice::destroyShader(ShaderId shader)
{
  if (m_backend) {
    m_backend->destroyShader(shader);
  }
}

PipelineId
GraphicsDevice::createPipeline(const PipelineCreateInfo& info)
{
  return m_backend ? m_backend->createPipeline(info) : PipelineId{};
}

void
GraphicsDevice::destroyPipeline(PipelineId pipeline)
{
  if (m_backend) {
    m_backend->destroyPipeline(pipeline);
  }
}

FramebufferId
GraphicsDevice::createFramebuffer(const FramebufferCreateInfo& info)
{
  return m_backend ? m_backend->createFramebuffer(info) : FramebufferId{};
}

void
GraphicsDevice::destroyFramebuffer(FramebufferId framebuffer)
{
  if (m_backend) {
    m_backend->destroyFramebuffer(framebuffer);
  }
}

void
GraphicsDevice::setFramebufferAttachment(FramebufferId fbo,
                                         u32 attachmentIndex,
                                         TextureId texture,
                                         u32 mipLevel,
                                         u32 layer)
{
  if (m_backend) {
    m_backend->setFramebufferAttachment(
      fbo, attachmentIndex, texture, mipLevel, layer);
  }
}

void
GraphicsDevice::setFramebufferDepthAttachment(FramebufferId fbo,
                                              TextureId texture,
                                              u32 mipLevel,
                                              u32 layer)
{
  if (m_backend) {
    m_backend->setFramebufferDepthAttachment(fbo, texture, mipLevel, layer);
  }
}

void
GraphicsDevice::setFramebufferRenderbuffer(
  FramebufferId fbo,
  RenderbufferAttachment attachmentType,
  RenderbufferId rbo)
{
  if (m_backend) {
    m_backend->setFramebufferRenderbuffer(fbo, attachmentType, rbo);
  }
}

void
GraphicsDevice::setDrawBuffers(FramebufferId fbo,
                               std::span<const u32> attachments)
{
  if (m_backend) {
    m_backend->setDrawBuffers(fbo, attachments);
  }
}

RenderbufferId
GraphicsDevice::createRenderbuffer(const RenderbufferCreateInfo& info)
{
  return m_backend ? m_backend->createRenderbuffer(info) : RenderbufferId{};
}

void
GraphicsDevice::destroyRenderbuffer(RenderbufferId renderbuffer)
{
  if (m_backend) {
    m_backend->destroyRenderbuffer(renderbuffer);
  }
}

void
GraphicsDevice::resizeRenderbuffer(RenderbufferId renderbuffer,
                                   u32 width,
                                   u32 height)
{
  if (m_backend) {
    m_backend->resizeRenderbuffer(renderbuffer, width, height);
  }
}

VertexArrayId
GraphicsDevice::createVertexArray(const VertexArrayCreateInfo& info)
{
  return m_backend ? m_backend->createVertexArray(info) : VertexArrayId{};
}

void
GraphicsDevice::destroyVertexArray(VertexArrayId vao)
{
  if (m_backend) {
    m_backend->destroyVertexArray(vao);
  }
}

void
GraphicsDevice::bindVertexArray(VertexArrayId vao)
{
  if (m_backend) {
    m_backend->bindVertexArray(vao);
  }
}

void
GraphicsDevice::drawVertexArray(VertexArrayId vao, BufferId vertexBuffer)
{
  if (m_backend) {
    m_backend->drawVertexArray(vao, vertexBuffer);
  }
}

void
GraphicsDevice::drawVertexArrayDynamic(VertexArrayId vao,
                                       BufferId vertexBuffer,
                                       PrimitiveTopology topology,
                                       u32 vertexCount)
{
  if (m_backend) {
    m_backend->drawVertexArrayDynamic(vao, vertexBuffer, topology, vertexCount);
  }
}

void
GraphicsDevice::drawIndexedVertexArray(VertexArrayId vao,
                                       BufferId vertexBuffer,
                                       BufferId indexBuffer,
                                       u32 indexCount,
                                       IndexType indexType,
                                       u32 offset)
{
  if (m_backend) {
    m_backend->drawIndexedVertexArray(
      vao, vertexBuffer, indexBuffer, indexCount, indexType, offset);
  }
}

void
GraphicsDevice::updateBuffer(BufferId buffer,
                             u64 offset,
                             const void* data,
                             u64 size)
{
  if (m_backend) {
    m_backend->updateBuffer(buffer, offset, data, size);
  }
}

void*
GraphicsDevice::mapBuffer(BufferId buffer)
{
  return m_backend ? m_backend->mapBuffer(buffer) : nullptr;
}

void
GraphicsDevice::unmapBuffer(BufferId buffer)
{
  if (m_backend) {
    m_backend->unmapBuffer(buffer);
  }
}

void
GraphicsDevice::updateTexture(TextureId texture,
                              u32 mipLevel,
                              u32 layer,
                              const void* data,
                              u64 dataSize)
{
  if (m_backend) {
    m_backend->updateTexture(texture, mipLevel, layer, data, dataSize);
  }
}

void
GraphicsDevice::resizeTexture(TextureId texture,
                              u32 newWidth,
                              u32 newHeight,
                              const void* data)
{
  if (m_backend) {
    m_backend->resizeTexture(texture, newWidth, newHeight, data);
  }
}

void
GraphicsDevice::generateMipmaps(TextureId texture)
{
  if (m_backend) {
    m_backend->generateMipmaps(texture);
  }
}

CommandBufferId
GraphicsDevice::createCommandBuffer()
{
  return m_backend ? m_backend->createCommandBuffer() : CommandBufferId{};
}

void
GraphicsDevice::destroyCommandBuffer(CommandBufferId cmdBuffer)
{
  if (m_backend) {
    m_backend->destroyCommandBuffer(cmdBuffer);
  }
}

CommandBuffer*
GraphicsDevice::getCommandBuffer(CommandBufferId cmdId)
{
  return m_backend ? m_backend->getCommandBuffer(cmdId) : nullptr;
}

void
GraphicsDevice::submit(CommandBufferId cmdBuffer)
{
  if (m_backend) {
    auto* cmd = m_backend->getCommandBuffer(cmdBuffer);
    if (cmd) {
      m_backend->executeCommandBuffer(*cmd);
    }
  }
}

void
GraphicsDevice::beginFrame()
{
  if (m_backend) {
    m_backend->beginFrame();
  }
}

void
GraphicsDevice::endFrame()
{
  if (m_backend) {
    m_backend->endFrame();
  }
}

bool
GraphicsDevice::isValid(BufferId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isValid(TextureId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isValid(SamplerId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isValid(ShaderId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isValid(PipelineId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isValid(FramebufferId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isFramebufferComplete(FramebufferId fbo) const
{
  return m_backend && m_backend->isFramebufferComplete(fbo);
}

bool
GraphicsDevice::isValid(RenderbufferId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

bool
GraphicsDevice::isValid(VertexArrayId handle) const
{
  return m_backend && m_backend->isValid(handle);
}

u32
GraphicsDevice::getNativeHandle(BufferId buffer) const
{
  return m_backend ? m_backend->getNativeHandle(buffer) : 0;
}

u32
GraphicsDevice::getNativeHandle(TextureId texture) const
{
  return m_backend ? m_backend->getNativeHandle(texture) : 0;
}

u32
GraphicsDevice::getNativeHandle(ShaderId program) const
{
  return m_backend ? m_backend->getNativeHandle(program) : 0;
}

u32
GraphicsDevice::getNativeHandle(FramebufferId framebuffer) const
{
  return m_backend ? m_backend->getNativeHandle(framebuffer) : 0;
}

u32
GraphicsDevice::getNativeHandle(RenderbufferId renderbuffer) const
{
  return m_backend ? m_backend->getNativeHandle(renderbuffer) : 0;
}

i32
GraphicsDevice::getUniformLocation(ShaderId program, const char* name) const
{
  return m_backend ? m_backend->getUniformLocation(program, name) : -1;
}

i32
GraphicsDevice::getAttribLocation(ShaderId program, const char* name) const
{
  return m_backend ? m_backend->getAttribLocation(program, name) : -1;
}

void
GraphicsDevice::setUniformInt(i32 location, i32 value)
{
  if (m_backend) {
    m_backend->setUniformInt(location, value);
  }
}

void
GraphicsDevice::setUniformIntArray(i32 location, std::span<const i32> values)
{
  if (m_backend) {
    m_backend->setUniformIntArray(location, values);
  }
}

void
GraphicsDevice::setUniformFloat(i32 location, float value)
{
  if (m_backend) {
    m_backend->setUniformFloat(location, value);
  }
}

void
GraphicsDevice::setUniformVec2(i32 location, const glm::vec2& value)
{
  if (m_backend) {
    m_backend->setUniformVec2(location, value);
  }
}

void
GraphicsDevice::setUniformVec3(i32 location, const glm::vec3& value)
{
  if (m_backend) {
    m_backend->setUniformVec3(location, value);
  }
}

void
GraphicsDevice::setUniformVec4(i32 location, const glm::vec4& value)
{
  if (m_backend) {
    m_backend->setUniformVec4(location, value);
  }
}

void
GraphicsDevice::setUniformMat3(i32 location, const glm::mat3& value)
{
  if (m_backend) {
    m_backend->setUniformMat3(location, value);
  }
}

void
GraphicsDevice::setUniformMat4(i32 location, const glm::mat4& value)
{
  if (m_backend) {
    m_backend->setUniformMat4(location, value);
  }
}

FramebufferId
GraphicsDevice::getDefaultFramebuffer() const
{
  return m_backend ? m_backend->getDefaultFramebuffer() : FramebufferId{};
}

void
GraphicsDevice::bindFramebuffer(FramebufferId fbo)
{
  if (m_backend) {
    m_backend->bindFramebuffer(fbo);
  }
}

void
GraphicsDevice::setViewport(i32 x, i32 y, u32 width, u32 height)
{
  if (m_backend) {
    m_backend->setViewport(x, y, width, height);
  }
}

void
GraphicsDevice::setScissor(i32 x, i32 y, u32 width, u32 height)
{
  if (m_backend) {
    m_backend->setScissor(x, y, width, height);
  }
}

void
GraphicsDevice::clearColor(std::optional<u32> attachment,
                           const glm::vec4& color)
{
  if (m_backend) {
    m_backend->clearColor(attachment, color);
  }
}

void
GraphicsDevice::clearDepth(float depth)
{
  if (m_backend) {
    m_backend->clearDepth(depth);
  }
}

void
GraphicsDevice::clearStencil(i32 value)
{
  if (m_backend) {
    m_backend->clearStencil(value);
  }
}

void
GraphicsDevice::clearDepthStencil(float depth, i32 stencil)
{
  if (m_backend) {
    m_backend->clearDepthStencil(depth, stencil);
  }
}

void
GraphicsDevice::setDepthTest(bool enable)
{
  if (m_backend) {
    m_backend->setDepthTest(enable);
  }
}

void
GraphicsDevice::setDepthFunc(CompareOp op)
{
  if (m_backend) {
    m_backend->setDepthFunc(op);
  }
}

void
GraphicsDevice::setDepthWrite(bool enable)
{
  if (m_backend) {
    m_backend->setDepthWrite(enable);
  }
}

void
GraphicsDevice::setCullMode(CullMode mode)
{
  if (m_backend) {
    m_backend->setCullMode(mode);
  }
}

void
GraphicsDevice::setFrontFace(FrontFace face)
{
  if (m_backend) {
    m_backend->setFrontFace(face);
  }
}

void
GraphicsDevice::setBlendEnabled(bool enable)
{
  if (m_backend) {
    m_backend->setBlendEnabled(enable);
  }
}

void
GraphicsDevice::setBlendFunc(BlendFactor srcColor,
                             BlendFactor dstColor,
                             BlendFactor srcAlpha,
                             BlendFactor dstAlpha)
{
  if (m_backend) {
    m_backend->setBlendFunc(srcColor, dstColor, srcAlpha, dstAlpha);
  }
}

void
GraphicsDevice::setBlendEquation(BlendOp colorOp, BlendOp alphaOp)
{
  if (m_backend) {
    m_backend->setBlendEquation(colorOp, alphaOp);
  }
}

void
GraphicsDevice::setScissorTest(bool enable)
{
  if (m_backend) {
    m_backend->setScissorTest(enable);
  }
}

void
GraphicsDevice::bindShaderProgram(ShaderId program)
{
  if (m_backend) {
    m_backend->bindShaderProgram(program);
  }
}

void
GraphicsDevice::bindTexture(u32 unit, TextureId texture)
{
  if (m_backend) {
    m_backend->bindTexture(unit, texture);
  }
}

void
GraphicsDevice::bindSampler(u32 unit, SamplerId sampler)
{
  if (m_backend) {
    m_backend->bindSampler(unit, sampler);
  }
}

void
GraphicsDevice::blitFramebuffer(FramebufferId srcFbo,
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
                                bool linearFilter)
{
  if (m_backend) {
    m_backend->blitFramebuffer(srcFbo,
                               dstFbo,
                               srcX0,
                               srcY0,
                               srcX1,
                               srcY1,
                               dstX0,
                               dstY0,
                               dstX1,
                               dstY1,
                               colorBit,
                               depthBit,
                               stencilBit,
                               linearFilter);
  }
}

void
GraphicsDevice::setSeamlessCubemap(bool enable)
{
  if (m_backend) {
    m_backend->setSeamlessCubemap(enable);
  }
}

void
GraphicsDevice::setLineWidth(float width)
{
  if (m_backend) {
    m_backend->setLineWidth(width);
  }
}

void
GraphicsDevice::setLineSmoothing(bool enable)
{
  if (m_backend) {
    m_backend->setLineSmoothing(enable);
  }
}

void
GraphicsDevice::setColorMask(bool red, bool green, bool blue, bool alpha)
{
  if (m_backend) {
    m_backend->setColorMask(red, green, blue, alpha);
  }
}

void
GraphicsDevice::setBlendColor(float red, float green, float blue, float alpha)
{
  if (m_backend) {
    m_backend->setBlendColor(red, green, blue, alpha);
  }
}

void
GraphicsDevice::setReadBuffer(std::optional<u32> attachment)
{
  if (m_backend) {
    m_backend->setReadBuffer(attachment);
  }
}

void
GraphicsDevice::bindUniformBuffer(u32 bindingPoint,
                                  BufferId buffer,
                                  u64 offset,
                                  u64 size)
{
  if (m_backend) {
    m_backend->bindUniformBuffer(bindingPoint, buffer, offset, size);
  }
}

void
GraphicsDevice::bindUniformBlock(ShaderId program,
                                 const char* blockName,
                                 u32 bindingPoint)
{
  if (m_backend) {
    m_backend->bindUniformBlock(program, blockName, bindingPoint);
  }
}

} // namespace gfx
