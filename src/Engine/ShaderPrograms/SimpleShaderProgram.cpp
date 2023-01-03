#include "SimpleShaderProgram.hpp"

#include <iostream>

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

SimpleShaderProgram::SimpleShaderProgram()
    : ShaderProgram("resources/Shaders/vertex.glsl",
                    "resources/Shaders/fragment.glsl") {
  m_uniformBindings["modelMatrix"] =
      glGetUniformLocation(p_shaderProgram, "modelMatrix");
  m_uniformBindings["viewMatrix"] =
      glGetUniformLocation(p_shaderProgram, "viewMatrix");
  m_uniformBindings["projMatrix"] =
      glGetUniformLocation(p_shaderProgram, "projMatrix");
  use();  // Start using the shader
}

SimpleShaderProgram::~SimpleShaderProgram() {}

void SimpleShaderProgram::setupVertexAttributePointers() {
  // Change if input layout changes in shaders
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (void *)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);

  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 9 * sizeof(float),
                        (void *)(7 * sizeof(float)));
  glEnableVertexAttribArray(2);
}
