#include "Primitive.hpp"

void
Primitive::draw()
{

  glBindVertexArray(m_vao);
  if (m_drawType == 0) {
    glDrawArrays(m_mode, 0, m_count);
  } else {
    glDrawElements(m_mode, m_count, m_type, (void*)(sizeof(char) * (m_offset)));
  }
}
