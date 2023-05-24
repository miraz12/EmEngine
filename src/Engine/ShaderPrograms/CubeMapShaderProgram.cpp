#include "CubeMapShaderProgram.hpp"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

CubeMapShaderProgram::CubeMapShaderProgram()
    : ShaderProgram("resources/Shaders/cubeVertex.glsl", "resources/Shaders/cubeFragment.glsl") {
  m_uniformBindings["viewMatrix"] = glGetUniformLocation(p_shaderProgram, "viewMatrix");
  m_uniformBindings["projMatrix"] = glGetUniformLocation(p_shaderProgram, "projMatrix");
  m_uniformBindings["equirectangularMap"] =
      glGetUniformLocation(p_shaderProgram, "equirectangularMap");
}
