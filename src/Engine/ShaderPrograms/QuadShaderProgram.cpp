#include "QuadShaderProgram.hpp"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <emscripten.h>
#define GL_GLEXT_PROTOTYPES
#else
#include <glad/glad.h>
#endif

QuadShaderProgram::QuadShaderProgram()
  : ShaderProgram("resources/Shaders/quadVertex.glsl",
                  "resources/Shaders/quadFragment.glsl")
{
  setUniformBinding("gPosition");
  setUniformBinding("gNormal");
  setUniformBinding("gAlbedoSpec");
  use(); // Start using the shader
}
