#include "MeshObj.hpp"
#include <glm/gtc/type_ptr.hpp>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

void MeshObj::draw() {

  glBindVertexArray(m_vao);
  if (m_drawType == 0) {
    glDrawArrays(m_mode, 0, m_count);
  } else {
    glDrawElements(m_mode, m_count, m_type, (void *)(sizeof(char) * (m_offset)));
  }
  glBindVertexArray(0);
}
