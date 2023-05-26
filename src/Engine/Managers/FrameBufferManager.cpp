#include "FrameBufferManager.hpp"

uint32_t FrameBufferManager::bindFBO(std::string name) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbos.at(name));
  return getFBO(name);
};
