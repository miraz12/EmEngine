#include "ScreenQuad.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

ScreenQuad::ScreenQuad(ShaderProgram &shaderProgram)
    : GraphicsObject(shaderProgram) {
  bindVAO();
  glGenBuffers(1, &m_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
  setVertexData(sizeof(m_quadVertices), m_quadVertices);
  p_shaderProgram.setupVertexAttributePointers();
  unbindVAO();
}

void ScreenQuad::draw() {
  bindVAO();

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, m_texture);

  glDrawArrays(GL_TRIANGLES, 0, 6);
}
