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
    : ShaderProgram("resources/Shaders/vertex.glsl", "resources/Shaders/fragment.glsl") {
  m_uniformBindings["modelMatrix"] = glGetUniformLocation(p_shaderProgram, "modelMatrix");
  m_uniformBindings["viewMatrix"] = glGetUniformLocation(p_shaderProgram, "viewMatrix");
  m_uniformBindings["projMatrix"] = glGetUniformLocation(p_shaderProgram, "projMatrix");
  use(); // Start using the shader
}

SimpleShaderProgram::~SimpleShaderProgram() {}
