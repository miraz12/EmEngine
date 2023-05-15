#include "Quad.hpp"

#include <iostream>

#include "ShaderPrograms/ShaderProgram.hpp"
#include "glm/fwd.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

Quad::Quad(ShaderProgram &/* shaderProgram */) {
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);

  glGenVertexArrays(1, &m_VAO);
  glBindVertexArray(m_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)(7 * sizeof(float)));
  glEnableVertexAttribArray(2);

  setVertexData(sizeof(m_vertices), m_vertices);
  setIndexData(sizeof(m_indices), m_indices);
}

Quad::~Quad() {
  glDeleteVertexArrays(1, &m_VAO);
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_EBO);
}

void Quad::setVertexData(std::size_t dataSize, const void *data) {
  glBindVertexArray(m_VAO);

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
}

void Quad::setIndexData(std::size_t dataSize, const void *data) {
  glBindVertexArray(m_VAO);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO),
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);
}

void Quad::draw(const ShaderProgram &sPrg) {
  sPrg.use();
  // cam.bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
  //                        p_shaderProgram.getUniformLocation("viewMatrix"));
  // glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1, GL_FALSE,
  //                    glm::value_ptr(model));
  glBindVertexArray(m_VAO);
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
