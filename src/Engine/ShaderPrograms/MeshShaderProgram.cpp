#include "MeshShaderProgram.hpp"

#include <iostream>

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

MeshShaderProgram::MeshShaderProgram()
    : ShaderProgram("resources/Shaders/meshVertex.glsl",
                    "resources/Shaders/meshFragment.glsl") {
  m_uniformBindings["modelMatrix"] =
      glGetUniformLocation(p_shaderProgram, "modelMatrix");
  m_uniformBindings["viewMatrix"] =
      glGetUniformLocation(p_shaderProgram, "viewMatrix");
  m_uniformBindings["projMatrix"] =
      glGetUniformLocation(p_shaderProgram, "projMatrix");
  use();  // Start using the shader
}

MeshShaderProgram::~MeshShaderProgram() {}
