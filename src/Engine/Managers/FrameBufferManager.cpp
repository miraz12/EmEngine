#include "FrameBufferManager.hpp"

u32
FrameBufferManager::bindFBO(std::string name)
{
  glBindFramebuffer(GL_FRAMEBUFFER, fbos.at(name));
  return getFBO(name);
};
