#include "Quad.hpp"
#include "ShaderPrograms/ShaderProgram.hpp"
#include <iostream>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

Quad::Quad() : GraphicsObject() {

  setVertexData(sizeof(m_vertices), m_vertices);
  setIndexData(sizeof(m_indices), m_indices);
}

Quad::~Quad() {}

void Quad::draw() {
  bindVAO();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
