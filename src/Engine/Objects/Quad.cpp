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

Quad::Quad(ShaderProgram& prog) : GraphicsObject(prog) {

  setVertexData(sizeof(m_vertices), m_vertices);
  setIndexData(sizeof(m_indices), m_indices);
}

Quad::~Quad() {}

void Quad::draw() {
  std::cout << "Render quad!" << std::endl;
  p_shaderProgram.use();
  bindVAO();
  glDrawArrays(GL_TRIANGLES, 0, 6);
}
