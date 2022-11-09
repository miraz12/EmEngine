#include "Quad.hpp"
#include "ShaderPrograms/ShaderProgram.hpp"
#include "glm/fwd.hpp"
#include <iostream>

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

Quad::Quad() : GraphicsObject() {

  m_model = glm::mat4(1.0f);
  setVertexData(sizeof(m_vertices), m_vertices);
  setIndexData(sizeof(m_indices), m_indices);
}

Quad::~Quad() {}

void Quad::draw(ShaderProgram& prog) {
  bindVAO();
  glUniformMatrix4fv(prog.getUniformLocation("modelMatrix"), 1,
                     GL_FALSE, glm::value_ptr(m_model));
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
