#include "Primitive.hpp"

void
Primitive::draw()
{

  glBindVertexArray(m_vao);

  if (m_drawType == 0) {
    glDrawArrays(m_mode, 0, m_count);
  } else {
    // Calculate offset based on index type
    size_t typeSize;
    switch (m_type) {
      case GL_UNSIGNED_SHORT:
        typeSize = sizeof(GLushort);
        break;
      case GL_UNSIGNED_INT:
        typeSize = sizeof(GLuint);
        break;
      default:
        typeSize = sizeof(GLubyte);
        break;
    }
    glDrawElements(m_mode, m_count, m_type, (void*)(typeSize * m_offset));
  }
}
