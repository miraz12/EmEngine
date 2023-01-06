#include "GraphicsObject.hpp"

#include "ShaderPrograms/ShaderProgram.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

GraphicsObject::GraphicsObject(ShaderProgram &shaderProgram)
    : p_shaderProgram(shaderProgram) {
  glGenVertexArrays(1, &m_VAO);
}

GraphicsObject::~GraphicsObject() { 
  glDeleteVertexArrays(1, &m_VAO);
  delete &p_shaderProgram;
 }

void GraphicsObject::bindVAO() { glBindVertexArray(m_VAO); }

void GraphicsObject::unbindVAO() { glBindVertexArray(0); }
