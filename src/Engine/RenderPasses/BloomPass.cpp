#include "BloomPass.hpp"
#include "RenderUtil.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

BloomPass::BloomPass()
  : RenderPass("resources/Shaders/quad.vert", "resources/Shaders/bloomUp.frag")
  , m_extractBright("resources/Shaders/quad.vert",
                    "resources/Shaders/extractBright.frag")
  , m_downShader("resources/Shaders/quad.vert",
                 "resources/Shaders/bloomDown.frag")
  , m_bloomCombine("resources/Shaders/quad.vert",
                   "resources/Shaders/bloomCombine.frag")
{

  u32 fbos[3];
  glGenFramebuffers(3, fbos);
  m_fboManager.setFBO("brightFBO", fbos[0]);
  m_fboManager.setFBO("bloomFBO", fbos[1]);
  m_fboManager.setFBO("bloomFinalFBO", fbos[2]);

  u32 frameBright;
  glGenTextures(1, &frameBright);
  m_textureManager.setTexture("frameBright", frameBright, GL_TEXTURE_2D);

  u32 frameBloomFinal;
  glGenTextures(1, &frameBloomFinal);
  m_textureManager.setTexture(
    "frameBloomFinal", frameBloomFinal, GL_TEXTURE_2D);

  glm::vec2 currentMipSize(m_width, m_height);
  glm::ivec2 currentMipSizeInt(m_width, m_height);
  for (u32 i = 0; i < 5; i++) {
    mipLevel mip;
    currentMipSize *= 0.5f;
    currentMipSizeInt /= 2;
    mip.size = currentMipSize;
    mip.intSize = currentMipSizeInt;

    glGenTextures(1, &mip.texture);
    m_mipChain.emplace_back(mip);
  }

  setViewport(m_width, m_height);

  m_shaderProgram.setAttribBinding("POSITION");
  m_shaderProgram.setAttribBinding("TEXCOORD_0");
  m_shaderProgram.setUniformBinding("srcResolution");
  m_shaderProgram.setUniformBinding("filterRadius");

  m_extractBright.setAttribBinding("POSITION");
  m_extractBright.setAttribBinding("TEXCOORD_0");
  m_extractBright.setUniformBinding("srcResolution");
  m_extractBright.setUniformBinding("mipLevel");

  m_downShader.setAttribBinding("POSITION");
  m_downShader.setAttribBinding("TEXCOORD_0");
  m_downShader.setUniformBinding("srcResolution");
  m_downShader.setUniformBinding("mipLevel");

  m_bloomCombine.setAttribBinding("POSITION");
  m_bloomCombine.setAttribBinding("TEXCOORD_0");
  m_bloomCombine.setUniformBinding("scene");
  m_bloomCombine.setUniformBinding("bloomBlur");
  m_bloomCombine.setUniformBinding("exposure");

  // Cache uniform locations for performance
  m_upFilterRadiusLoc = m_shaderProgram.getUniformLocation("filterRadius");
  m_downSrcResolutionLoc = m_downShader.getUniformLocation("srcResolution");
  m_downMipLevelLoc = m_downShader.getUniformLocation("mipLevel");
  m_combineExposureLoc = m_bloomCombine.getUniformLocation("exposure");
  m_combineSceneLoc = m_bloomCombine.getUniformLocation("scene");
  m_combineBloomBlurLoc = m_bloomCombine.getUniformLocation("bloomBlur");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
BloomPass::Execute(ECSManager& /* eManager */)
{
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Bloom Pass");
#endif
  m_fboManager.bindFBO("brightFBO");
  m_extractBright.use();
  m_textureManager.bindActivateTexture("cubeFrame", 0);
  Util::renderQuad();

  m_fboManager.bindFBO("bloomFBO");
  m_downShader.use();
  // Disable blending
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glUniform2f(m_downSrcResolutionLoc, m_width, m_height);
  glUniform1i(m_downMipLevelLoc, 0);

  // Bind srcTexture (HDR color buffer) as initial texture input
  m_textureManager.bindActivateTexture("frameBright", 0);

  // Progressively downsample through the mip chain
  for (u32 i = 0; i < (u32)m_mipChain.size(); i++) {
    const mipLevel& mip = m_mipChain[i];
    glViewport(0, 0, mip.size.x, mip.size.y);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

    Util::renderQuad();

    // Set current mip resolution as srcResolution for next iteration
    glUniform2f(m_downSrcResolutionLoc, mip.size.x, mip.size.y);
    // Set current mip as texture input for next iteration
    glBindTexture(GL_TEXTURE_2D, mip.texture);
    // Disable Karis average for consequent downsamples
    if (i == 0) {
      glUniform1i(m_downMipLevelLoc, 1);
    }
  }

  m_shaderProgram.use();
  glUniform1f(m_upFilterRadiusLoc, 0.005f);

  // Enable additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  for (u32 i = (u32)m_mipChain.size() - 1; i > 0; i--) {
    const mipLevel& mip = m_mipChain[i];
    const mipLevel& nextMip = m_mipChain[i - 1];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mip.texture);

    glViewport(0, 0, nextMip.size.x, nextMip.size.y);
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

    Util::renderQuad();
  }

  // Disable additive blending
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);

  m_fboManager.bindFBO("bloomFinalFBO");
  glViewport(0, 0, m_width, m_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_bloomCombine.use();
  glUniform1f(m_combineExposureLoc, 1.0f);
  glUniform1i(m_combineSceneLoc, 0);
  glUniform1i(m_combineBloomBlurLoc, 1);
  m_textureManager.bindActivateTexture("cubeFrame", 0);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, m_mipChain.front().texture);

  Util::renderQuad();
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPopDebugGroup();
#endif
}

void
BloomPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  m_fboManager.bindFBO("brightFBO");
  u32 frameBright = m_textureManager.bindTexture("frameBright");
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA16F,
               m_width,
               m_height,
               0,
               GL_RGBA,
               GL_FLOAT,
               NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_WRAP_S,
    GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would
                       // otherwise sample repeated texture values!
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBright, 0);

  u32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);
  // check completion status
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  m_fboManager.bindFBO("bloomFBO");
  for (u32 i = 0; i < 5; i++) {
    mipLevel mip = m_mipChain[i];

    glBindTexture(GL_TEXTURE_2D, mip.texture);
    // we are downscaling a HDR color buffer, so we need a float texture format
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_R11F_G11F_B10F,
                 (i32)mip.size.x,
                 (i32)mip.size.y,
                 0,
                 GL_RGB,
                 GL_FLOAT,
                 nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D,
                         m_mipChain[0].texture,
                         0);

  glDrawBuffers(1, attachments);
  // check completion status
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
    assert(false);
  }

  m_fboManager.bindFBO("bloomFinalFBO");
  u32 frameBloomFinal = m_textureManager.bindTexture("frameBloomFinal");
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGBA16F,
               m_width,
               m_height,
               0,
               GL_RGBA,
               GL_FLOAT,
               NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(
    GL_TEXTURE_2D,
    GL_TEXTURE_WRAP_S,
    GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would
                       // otherwise sample repeated texture values!
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBloomFinal, 0);
  glGenerateMipmap(GL_TEXTURE_2D);

  glDrawBuffers(1, attachments);
  // check completion status
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }
}
