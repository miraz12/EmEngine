#include "FxaaPass.hpp"
#include <RenderPasses/RenderUtil.hpp>

FxaaPass::FxaaPass()
  : RenderPass("resources/Shaders/quad.vert", "resources/Shaders/Fxaa.frag")
{
  m_shaderProgram.setAttribBinding("POSITION");
  m_shaderProgram.setAttribBinding("TEXCOORD_0");
  m_shaderProgram.setUniformBinding("scene");
  m_shaderProgram.setUniformBinding("resolution");

  // Cache uniform locations for performance
  m_sceneLoc = m_shaderProgram.getUniformLocation("scene");
  m_resolutionLoc = m_shaderProgram.getUniformLocation("resolution");
}

void
FxaaPass::Execute(ECSManager& /* eManager */)
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  // m_fboManager.bindFBO("FxaaFBO");
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
  m_shaderProgram.use();
  glUniform1i(m_sceneLoc, 0);
  glUniform2f(m_resolutionLoc, m_width, m_height);
  m_textureManager.bindActivateTexture("frameBloomFinal", 0);
  Util::renderQuad();
}

void
FxaaPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;
}
