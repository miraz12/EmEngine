#include "MeshObj.hpp"
#include <glm/gtc/type_ptr.hpp>

void MeshObj::draw() {

  glBindVertexArray(m_vao);
  if (m_drawType == 0) {
    glDrawArrays(m_mode, 0, m_count);
  } else {
    glDrawElements(m_mode, m_count, m_type, (void *)(sizeof(char) * (m_offset)));
  }
  glBindVertexArray(0);
}
