#include "QuadShaderProgram.hpp"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

QuadShaderProgram::QuadShaderProgram()
  : ShaderProgram("resources/Shaders/quad.vert", "resources/Shaders/quad.frag")
{
  setUniformBinding("gPosition");
  setUniformBinding("gNormal");
  setUniformBinding("gAlbedoSpec");
  use(); // Start using the shader
}
