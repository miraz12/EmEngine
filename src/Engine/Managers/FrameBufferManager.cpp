#include "FrameBufferManager.hpp"



unsigned int FrameBufferManager::bindFBO(std::string name) {
  glBindFramebuffer(GL_FRAMEBUFFER, fbos.at(name));
  return getFBO(name);
};
