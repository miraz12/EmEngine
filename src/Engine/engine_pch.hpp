#ifndef ENGINE_PCH_
#define ENGINE_PCH_

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

#include <cstdint>
#include <memory>

#endif // ENGINE_PCH_
