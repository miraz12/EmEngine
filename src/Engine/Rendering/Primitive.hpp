#ifndef PRIMITIVE_H_
#define PRIMITIVE_H_

#include <Graphics/GraphicsDevice.hpp>

namespace gfx {
class CommandBuffer;
}

class Primitive
{
public:
  void draw();
  /// Record draw command into the given CommandBuffer
  void recordDraw(gfx::CommandBuffer& cmd);

  i32 m_material{ -1 };

  // Graphics handles (abstracted)
  gfx::VertexArrayId m_vaoId{};
  gfx::BufferId m_vboId{};
  gfx::BufferId m_eboId{};
  gfx::PrimitiveTopology m_topology{ gfx::PrimitiveTopology::Triangles };
  gfx::IndexType m_indexType{ gfx::IndexType::U32 };

  u32 m_count{ 0 };
  u32 m_offset{ 0 };
};

#endif // PRIMITIVE_H_
