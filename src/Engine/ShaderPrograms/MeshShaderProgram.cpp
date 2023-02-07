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
  use();  // Start using the shader
  m_uniformBindings["modelMatrix"] =
      glGetUniformLocation(p_shaderProgram, "modelMatrix");
  m_uniformBindings["viewMatrix"] =
      glGetUniformLocation(p_shaderProgram, "viewMatrix");
  m_uniformBindings["projMatrix"] =
      glGetUniformLocation(p_shaderProgram, "projMatrix");
  m_uniformBindings["textures"] =
      glGetUniformLocation(p_shaderProgram, "textures");

  m_attribBindings["POSITION"] =
      glGetAttribLocation(p_shaderProgram, "POSITION");
  m_attribBindings["NORMAL"] = glGetAttribLocation(p_shaderProgram, "NORMAL");
  m_attribBindings["TANGENT"] = glGetAttribLocation(p_shaderProgram, "TANGENT");
  m_attribBindings["TEXCOORD_0"] =
      glGetAttribLocation(p_shaderProgram, "TEXCOORD_0");
}

MeshShaderProgram::~MeshShaderProgram() {}
