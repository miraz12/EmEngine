#include "FxaaPass.hpp"
#include <RenderPasses/RenderUtil.hpp>

FxaaPass::FxaaPass()
    : RenderPass("resources/Shaders/vertex2D.glsl", "resources/Shaders/FxaaFragment.glsl") {

  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("TEXCOORD_0");
  p_shaderProgram.setUniformBinding("scene");
  p_shaderProgram.setUniformBinding("resolution");

  uint32_t fbo;
  glGenFramebuffers(1, &fbo);
  p_fboManager.setFBO("FxaaFBO", fbo);
  uint32_t frameFxaa;
  glGenTextures(1, &frameFxaa);
  p_textureManager.setTexture("frameFxaa", frameFxaa, GL_TEXTURE_2D);
  setViewport(p_width, p_height);
}

void FxaaPass::Execute(ECSManager & /* eManager */) {
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // p_fboManager.bindFBO("FxaaFBO");
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  p_shaderProgram.use();
  glUniform1i(p_shaderProgram.getUniformLocation("scene"), 0);

  glUniform2f(p_shaderProgram.getUniformLocation("resolution"), p_width, p_height);
  p_textureManager.bindActivateTexture("frameBloomFinal", 0);
  Util::renderQuad();
}

void FxaaPass::setViewport(uint32_t w, uint32_t h) {
  p_width = w;
  p_height = h;

  p_fboManager.bindFBO("FxaaFBO");
  uint32_t frameFxaa = p_textureManager.bindTexture("frameFxaa");
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, p_width, p_height, 0, GL_RGBA, GL_FLOAT, nullptr);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameFxaa, 0);
  uint32_t attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
  // check completion status
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }
}
