#include "BloomPass.hpp"
#include "FrameGraph.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include <iostream>

BloomPass::BloomPass()
    : RenderPass("resources/Shaders/vertex2D.glsl", "resources/Shaders/bloomUpFragment.glsl"),
      m_extractBright("resources/Shaders/vertex2D.glsl",
                      "resources/Shaders/extractBrightFragment.glsl"),
      m_downShader("resources/Shaders/vertex2D.glsl", "resources/Shaders/bloomDownFragment.glsl"),
      m_bloomCombine("resources/Shaders/vertex2D.glsl",
                     "resources/Shaders/bloomCombineFragment.glsl") {

  uint32_t fbos[2];
  glGenFramebuffers(2, fbos);
  p_fboManager.setFBO("brightFBO", fbos[0]);
  p_fboManager.setFBO("bloomFBO", fbos[1]);

  p_fboManager.bindFBO("brightFBO");
  uint32_t frameBright;
  glGenTextures(1, &frameBright);
  p_textureManager.setTexture("frameBright", frameBright, GL_TEXTURE_2D);

  glm::vec2 currentMipSize(p_width, p_height);
  glm::ivec2 currentMipSizeInt(p_width, p_height);
  for (unsigned int i = 0; i < 5; i++) {
    mipLevel mip;
    currentMipSize *= 0.5f;
    currentMipSizeInt /= 2;
    mip.size = currentMipSize;
    mip.intSize = currentMipSizeInt;

    glGenTextures(1, &mip.texture); 
    m_mipChain.emplace_back(mip);
  }

  setViewport(p_width, p_height);

  p_shaderProgram.setAttribBinding("POSITION");
  p_shaderProgram.setAttribBinding("TEXCOORD_0");
  p_shaderProgram.setUniformBinding("srcResolution");
  p_shaderProgram.setUniformBinding("filterRadius");

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

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void BloomPass::Execute(ECSManager & /* eManager */) {

  p_fboManager.bindFBO("brightFBO");
  m_extractBright.use();
  p_textureManager.bindActivateTexture("cubeFrame", 0);
  renderQuad();

  p_fboManager.bindFBO("bloomFBO");
  m_downShader.use();
  // Disable blending
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  glUniform2f(m_downShader.getUniformLocation("srcResolution"), p_width, p_height);

  if (true) {
    glUniform1i(m_downShader.getUniformLocation("mipLevel"), 0);
  }

  // Bind srcTexture (HDR color buffer) as initial texture input
  p_textureManager.bindActivateTexture("frameBright", 0);

  // Progressively downsample through the mip chain
  for (int i = 0; i < (int)m_mipChain.size(); i++) {
    const mipLevel &mip = m_mipChain[i];
    glViewport(0, 0, mip.size.x, mip.size.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);

    // Render screen-filled quad of resolution of current mip
    renderQuad();

    // Set current mip resolution as srcResolution for next iteration
    glUniform2f(m_downShader.getUniformLocation("srcResolution"), mip.size.x, mip.size.y);
    // Set current mip as texture input for next iteration
    glBindTexture(GL_TEXTURE_2D, mip.texture);
    // Disable Karis average for consequent downsamples
    if (i == 0) {
      glUniform1i(m_downShader.getUniformLocation("mipLevel"), 1);
    }
  }

  p_shaderProgram.use();
  glUniform1f(p_shaderProgram.getUniformLocation("filterRadius"), 0.005f);

  // Enable additive blending
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  for (int i = (int)m_mipChain.size() - 1; i > 0; i--) {
    const mipLevel &mip = m_mipChain[i];
    const mipLevel &nextMip = m_mipChain[i - 1];

    // Bind viewport and texture from where to read
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mip.texture);

    // Set framebuffer render target (we write to this texture)
    glViewport(0, 0, nextMip.size.x, nextMip.size.y);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, nextMip.texture, 0);

    // Render screen-filled quad of resolution of current mip
    renderQuad();
  }

  // Disable additive blending
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glViewport(0, 0, p_width, p_height);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  m_bloomCombine.use();
  glUniform1f(m_bloomCombine.getUniformLocation("exposure"), 1.0f);
  glUniform1i(m_bloomCombine.getUniformLocation("scene"), 0);
  glUniform1i(m_bloomCombine.getUniformLocation("bloomBlur"), 1);
  p_textureManager.bindActivateTexture("cubeFrame", 0);
  glActiveTexture(GL_TEXTURE0 + 1);
  glBindTexture(GL_TEXTURE_2D, m_mipChain.front().texture);

  renderQuad();
}

void BloomPass::setViewport(uint32_t w, uint32_t h) {
  p_width = w;
  p_height = h;

  p_fboManager.bindFBO("brightFBO");
  uint32_t frameBright = p_textureManager.bindTexture("frameBright");
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, p_width, p_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise
                                     // sample repeated texture values!
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, frameBright, 0);

  uint32_t attachments[1] = {GL_COLOR_ATTACHMENT0};
  glDrawBuffers(1, attachments);
  // check completion status
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  p_fboManager.bindFBO("bloomFBO");
  for (unsigned int i = 0; i < 5; i++) {
    mipLevel mip = m_mipChain[i];

    glBindTexture(GL_TEXTURE_2D, mip.texture);
    // we are downscaling a HDR color buffer, so we need a float texture format
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R11F_G11F_B10F, (int)mip.size.x, (int)mip.size.y,
                 0, GL_RGB, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_mipChain[0].texture,
                         0);

  glDrawBuffers(1, attachments);
  // check completion status
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
    assert(false);
  }
}

void BloomPass::renderQuad() {
  if (m_quadVAO == 0) {
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f, 1.0f,  -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // setup plane VAO
    glGenVertexArrays(1, &m_quadVAO);
    glGenBuffers(1, &m_quadVBO);
    glBindVertexArray(m_quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
  }
  glBindVertexArray(m_quadVAO);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
