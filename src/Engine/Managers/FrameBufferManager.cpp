#include "FrameBufferManager.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

unsigned int FrameBufferManager::bindFBO(std::string name) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbos.at(name));
  return getFBO(name);
};
