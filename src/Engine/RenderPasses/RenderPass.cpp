#include "RenderPass.hpp"

RenderPass::RenderPass(std::string vs, std::string fs)
    : p_shaderProgram(vs, fs), p_fboManager(FrameBufferManager::getInstance()),
      p_textureManager(TextureManager::getInstance()) {}
