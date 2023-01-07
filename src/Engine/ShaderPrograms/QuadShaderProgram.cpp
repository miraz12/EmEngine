#include "QuadShaderProgram.hpp"

#include <iostream>

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

QuadShaderProgram::QuadShaderProgram()
    : ShaderProgram("resources/Shaders/quadVertex.glsl",
                    "resources/Shaders/quadFragment.glsl") {
  m_uniformBindings["gPosition"] =
      glGetUniformLocation(p_shaderProgram, "gPosition");
  m_uniformBindings["gNormal"] =
      glGetUniformLocation(p_shaderProgram, "gNormal");
  m_uniformBindings["gAlbedoSpec"] =
      glGetUniformLocation(p_shaderProgram, "gAlbedoSpec");
  use(); // Start using the shader
}

QuadShaderProgram::~QuadShaderProgram() {}