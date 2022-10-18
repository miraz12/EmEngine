#include "GraphicsObject.hpp"

#include "ShaderPrograms/ShaderProgram.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

GraphicsObject::GraphicsObject() {
  glGenVertexArrays(1, &m_VAO);
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);

  glBindVertexArray(m_VAO);
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  glBindVertexArray(0);
}

GraphicsObject::~GraphicsObject() {
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_EBO);
}

void GraphicsObject::bindVAO() { glBindVertexArray(m_VAO); }

void GraphicsObject::unbindVAO() { glBindVertexArray(0); }

void GraphicsObject::setVertexData(std::size_t dataSize, const void *data) {
  glBindVertexArray(m_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void GraphicsObject::setIndexData(std::size_t dataSize, const void *data) {
  glBindVertexArray(m_VAO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO),
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

  glBindVertexArray(0);
}
