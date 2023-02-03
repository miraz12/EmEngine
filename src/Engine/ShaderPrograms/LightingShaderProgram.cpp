#include "LightingShaderProgram.hpp"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

LightingShaderProgram::LightingShaderProgram()
    : ShaderProgram("resources/Shaders/lightVertex.glsl",
                    "resources/Shaders/lightFragment.glsl") {
  m_uniformBindings["gPosition"] =
      glGetUniformLocation(p_shaderProgram, "gPosition");
  m_uniformBindings["gNormal"] =
      glGetUniformLocation(p_shaderProgram, "gNormal");
  m_uniformBindings["gAlbedoSpec"] =
      glGetUniformLocation(p_shaderProgram, "gAlbedoSpec");
  use(); // Start using the shader
}
