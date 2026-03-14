#include "FxaaPass.hpp"
#include <RenderPasses/RenderUtil.hpp>

FxaaPass::FxaaPass()
  : RenderPass("resources/Shaders/quad.vert", "resources/Shaders/Fxaa.frag")
{
  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("TEXCOORD_0");
  p_shaderProgram.setUniformBinding("scene");
  p_shaderProgram.setUniformBinding("resolution");
}

void
FxaaPass::Execute(ECSManager& /* eManager */)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // p_fboManager.bindFBO("FxaaFBO");
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  p_shaderProgram.use();
  glUniform1i(p_shaderProgram.getUniformLocation("scene"), 0);

  glUniform2f(
    p_shaderProgram.getUniformLocation("resolution"), p_width, p_height);
  p_textureManager.bindActivateTexture("frameBloomFinal", 0);
  Util::renderQuad();
}

void
FxaaPass::setViewport(u32 w, u32 h)
{
  p_width = w;
  p_height = h;
}
