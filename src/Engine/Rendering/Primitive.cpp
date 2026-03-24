#include "Primitive.hpp"
#include <Graphics/CommandBuffer.hpp>

void
Primitive::draw()
{
  auto& device = gfx::GraphicsDevice::getInstance();

  if (device.isValid(m_eboId)) {
    // Indexed draw
    device.drawIndexedVertexArray(
      m_vaoId, m_vboId, m_eboId, m_count, m_indexType, m_offset);
  } else {
    // Non-indexed draw
    device.drawVertexArray(m_vaoId, m_vboId);
  }
}

void
Primitive::recordDraw(gfx::CommandBuffer& cmd)
{
  auto& device = gfx::GraphicsDevice::getInstance();

  // Bind the VAO for this primitive (each primitive has its own vertex layout)
  cmd.bindVertexArray(m_vaoId);

  if (device.isValid(m_eboId)) {
    // Indexed draw - bind vertex and index buffers, then draw
    cmd.bindVertexBuffer(0, m_vboId);
    cmd.bindIndexBuffer(m_eboId, 0, m_indexType);
    cmd.drawIndexed(m_count, 1, m_offset);
  } else {
    // Non-indexed draw
    cmd.bindVertexBuffer(0, m_vboId);
    cmd.draw(m_count);
  }
}
