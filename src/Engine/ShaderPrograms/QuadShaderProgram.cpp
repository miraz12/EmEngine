#include "QuadShaderProgram.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

QuadShaderProgram::QuadShaderProgram()
    : ShaderProgram("resources/Shaders/vertex_quad.glsl",
                    "resources/Shaders/fragment_quad.glsl") {
  glUniform1i(m_uniformBindings["screenTexture"], 0);
}

void QuadShaderProgram::setupVertexAttributePointers() {
  // Change if input layout changes in shaders
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                        (void *)(2 * sizeof(float)));
  glEnableVertexAttribArray(1);
}
