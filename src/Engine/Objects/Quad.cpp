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

Quad::Quad(ShaderProgram &shaderProgram) : GraphicsObject(shaderProgram) {
  glGenBuffers(1, &m_VBO);
  glGenBuffers(1, &m_EBO);

  bindVAO();
  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (void *)(7 * sizeof(float)));
  glEnableVertexAttribArray(2);
  glBindVertexArray(0);

  m_model = glm::mat4(1.0f);
  setVertexData(sizeof(m_vertices), m_vertices);
  setIndexData(sizeof(m_indices), m_indices);
}

Quad::~Quad() {
  glDeleteBuffers(1, &m_VBO);
  glDeleteBuffers(1, &m_EBO);
}

void Quad::setVertexData(std::size_t dataSize, const void *data) {
  bindVAO();

  glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
  glBufferData(GL_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Quad::setIndexData(std::size_t dataSize, const void *data) {
  bindVAO();

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO),
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, dataSize, data, GL_STATIC_DRAW);

  glBindVertexArray(0);
}

void Quad::draw(Camera &cam) {
  p_shaderProgram.use();
  cam.bindProjViewMatrix(p_shaderProgram.getUniformLocation("projMatrix"),
                         p_shaderProgram.getUniformLocation("viewMatrix"));
  glUniformMatrix4fv(p_shaderProgram.getUniformLocation("modelMatrix"), 1,
                     GL_FALSE, glm::value_ptr(m_model));
  bindVAO();
  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
