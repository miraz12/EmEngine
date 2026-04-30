#include "CommandBuffer.hpp"
#include <cstring>

namespace gfx {

template<typename T>
void
CommandBuffer::encode(const T& value)
{
  const auto* bytes = reinterpret_cast<const u8*>(&value);
  m_commandStream.insert(m_commandStream.end(), bytes, bytes + sizeof(T));
}

void
CommandBuffer::encodeCommand(CommandType cmd)
{
  encode(cmd);
}

void
CommandBuffer::beginRenderPass(const RenderPassBeginInfo& info)
{
  encodeCommand(CommandType::BeginRenderPass);
  encode(info);
}

void
CommandBuffer::endRenderPass()
{
  encodeCommand(CommandType::EndRenderPass);
}

void
CommandBuffer::bindPipeline(PipelineId pipeline)
{
  encodeCommand(CommandType::BindPipeline);
  encode(pipeline);
}

void
CommandBuffer::setViewport(const Viewport& viewport)
{
  encodeCommand(CommandType::SetViewport);
  encode(viewport);
}

void
CommandBuffer::setScissor(const ScissorRect& scissor)
{
  encodeCommand(CommandType::SetScissor);
  encode(scissor);
}

void
CommandBuffer::bindVertexArray(VertexArrayId vao)
{
  encodeCommand(CommandType::BindVertexArray);
  encode(vao);
}

void
CommandBuffer::bindVertexBuffer(u32 binding, BufferId buffer, u64 offset)
{
  encodeCommand(CommandType::BindVertexBuffer);
  encode(binding);
  encode(buffer);
  encode(offset);
}

void
CommandBuffer::bindIndexBuffer(BufferId buffer, u64 offset, IndexType indexType)
{
  encodeCommand(CommandType::BindIndexBuffer);
  encode(buffer);
  encode(offset);
  encode(indexType);
}

void
CommandBuffer::bindUniformBuffer(u32 binding,
                                 BufferId buffer,
                                 u64 offset,
                                 u64 size)
{
  encodeCommand(CommandType::BindUniformBuffer);
  encode(binding);
  encode(buffer);
  encode(offset);
  encode(size);
}

void
CommandBuffer::bindTexture(u32 slot, TextureId texture, SamplerId sampler)
{
  encodeCommand(CommandType::BindTexture);
  encode(slot);
  encode(texture);
  encode(sampler);
}

void
CommandBuffer::bindTextureByName(u32 slot,
                                 const std::string& textureName,
                                 SamplerId sampler)
{
  encodeCommand(CommandType::BindTextureByName);
  encode(slot);
  // Encode string length followed by characters
  u32 len = static_cast<u32>(textureName.size());
  encode(len);
  m_commandStream.insert(
    m_commandStream.end(), textureName.begin(), textureName.end());
  encode(sampler);
}

void
CommandBuffer::setFramebufferAttachment(FramebufferId fbo,
                                        u32 attachmentIndex,
                                        TextureId texture,
                                        u32 mipLevel,
                                        u32 layer)
{
  encodeCommand(CommandType::SetFramebufferAttachment);
  encode(fbo);
  encode(attachmentIndex);
  encode(texture);
  encode(mipLevel);
  encode(layer);
}

void
CommandBuffer::setFramebufferDepthAttachment(FramebufferId fbo,
                                             TextureId texture,
                                             u32 mipLevel,
                                             u32 layer)
{
  encodeCommand(CommandType::SetFramebufferDepthAttachment);
  encode(fbo);
  encode(texture);
  encode(mipLevel);
  encode(layer);
}

void
CommandBuffer::clearDepth(float depth)
{
  encodeCommand(CommandType::ClearDepth);
  encode(depth);
}

void
CommandBuffer::setCullMode(CullMode mode)
{
  encodeCommand(CommandType::SetCullMode);
  encode(mode);
}

void
CommandBuffer::setBlendEnabled(bool enabled)
{
  encodeCommand(CommandType::SetBlendEnabled);
  encode(enabled);
}

void
CommandBuffer::setBlendFunc(BlendFactor srcColor,
                            BlendFactor dstColor,
                            BlendFactor srcAlpha,
                            BlendFactor dstAlpha)
{
  encodeCommand(CommandType::SetBlendFunc);
  encode(srcColor);
  encode(dstColor);
  encode(srcAlpha);
  encode(dstAlpha);
}

void
CommandBuffer::flushMaterialUBO()
{
  encodeCommand(CommandType::FlushMaterialUBO);
}

void
CommandBuffer::updateMaterialUBO(const MaterialUBO& data)
{
  encodeCommand(CommandType::UpdateMaterialUBO);
  encode(data);
}

void
CommandBuffer::setUniform(i32 location, const glm::mat4& value)
{
  encodeCommand(CommandType::SetUniformMat4);
  encode(location);
  encode(value);
}

void
CommandBuffer::setUniform(i32 location, const glm::vec4& value)
{
  encodeCommand(CommandType::SetUniformVec4);
  encode(location);
  encode(value);
}

void
CommandBuffer::setUniform(i32 location, const glm::vec3& value)
{
  encodeCommand(CommandType::SetUniformVec3);
  encode(location);
  encode(value);
}

void
CommandBuffer::setUniform(i32 location, const glm::vec2& value)
{
  encodeCommand(CommandType::SetUniformVec2);
  encode(location);
  encode(value);
}

void
CommandBuffer::setUniform(i32 location, float value)
{
  encodeCommand(CommandType::SetUniformFloat);
  encode(location);
  encode(value);
}

void
CommandBuffer::setUniform(i32 location, i32 value)
{
  encodeCommand(CommandType::SetUniformInt);
  encode(location);
  encode(value);
}

void
CommandBuffer::draw(u32 vertexCount,
                    u32 instanceCount,
                    u32 firstVertex,
                    u32 firstInstance)
{
  encodeCommand(CommandType::Draw);
  encode(vertexCount);
  encode(instanceCount);
  encode(firstVertex);
  encode(firstInstance);
}

void
CommandBuffer::drawIndexed(u32 indexCount,
                           u32 instanceCount,
                           u32 firstIndex,
                           i32 vertexOffset,
                           u32 firstInstance)
{
  encodeCommand(CommandType::DrawIndexed);
  encode(indexCount);
  encode(instanceCount);
  encode(firstIndex);
  encode(vertexOffset);
  encode(firstInstance);
}

void
CommandBuffer::blitFramebuffer(FramebufferId src,
                               FramebufferId dst,
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
  encodeCommand(CommandType::BlitFramebuffer);
  encode(src);
  encode(dst);
  encode(srcX0);
  encode(srcY0);
  encode(srcX1);
  encode(srcY1);
  encode(dstX0);
  encode(dstY0);
  encode(dstX1);
  encode(dstY1);
  encode(colorBit);
  encode(depthBit);
  encode(stencilBit);
  encode(linearFilter);
}

void
CommandBuffer::pushDebugGroup(const char* name)
{
  encodeCommand(CommandType::PushDebugGroup);
  u32 nameIndex = static_cast<u32>(m_debugNames.size());
  m_debugNames.emplace_back(name);
  encode(nameIndex);
}

void
CommandBuffer::popDebugGroup()
{
  encodeCommand(CommandType::PopDebugGroup);
}

void
CommandBuffer::reset()
{
  m_commandStream.clear();
  m_debugNames.clear();
}

} // namespace gfx
