#include "GraphicsObject.hpp"

#include "ShaderPrograms/ShaderProgram.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

GraphicsObject::GraphicsObject(ShaderProgram &shaderProgram) : p_shaderProgram(shaderProgram) {}

GraphicsObject::~GraphicsObject() { delete &p_shaderProgram; }
