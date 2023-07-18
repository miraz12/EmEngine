#include "CubeMapPass.hpp"

#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/fwd.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"

#include <ECS/ECSManager.hpp>
#include <RenderPasses/FrameGraph.hpp>

CubeMapPass::CubeMapPass()
    : RenderPass("resources/Shaders/backgroundVertex.glsl",
                 "resources/Shaders/backgroundFragment.glsl"),
      m_equirectangularToCubemapShader("resources/Shaders/cubeMapVertex.glsl",
                                       "resources/Shaders/equirectangularToCubemapFragment.glsl"),
      m_irradianceShader("resources/Shaders/cubeMapVertex.glsl",
                         "resources/Shaders/irradianceFragment.glsl"),
      m_prefilterShader("resources/Shaders/cubeMapVertex.glsl",
                        "resources/Shaders/prefilterFragment.glsl"),
      m_brdfShader("resources/Shaders/quadVertex.glsl", "resources/Shaders/brdfFragment.glsl") {

  stbi_set_flip_vertically_on_load(true);
  int32_t width, height, nrComponents;
  float *data =
      stbi_loadf("resources/Textures/clarens_midday_1k.hdr", &width, &height, &nrComponents, 0);
  uint32_t hdrTexture;
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    p_textureManager.setTexture("hdrTexture", hdrTexture);

    stbi_image_free(data);
  } else {
    std::cout << "Failed to load HDR image." << std::endl;
  }
  stbi_set_flip_vertically_on_load(false);

  glGenFramebuffers(1, &m_captureFBO);
  p_fboManager.setFBO("iblCaptureFBO", m_captureFBO);
  glGenRenderbuffers(1, &m_captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);

  generateCubeMap();
  generateIrradianceMap();
  generatePrefilterMap();
  generateBRDF();

  p_shaderProgram.setUniformBinding("projection");
  p_shaderProgram.setUniformBinding("view");

  glGenFramebuffers(1, &m_cubeBuffer);
  glGenRenderbuffers(1, &m_rbo);
  p_fboManager.setFBO("cubeFBO", m_cubeBuffer);
  glViewport(0, 0, p_width, p_height);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void CubeMapPass::Init(FrameGraph &fGraph) {
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("irradianceMap");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("prefilterMap");
  fGraph.m_renderPass[static_cast<size_t>(PassId::kLight)]->addTexture("brdfLUT");
}

void CubeMapPass::Execute(ECSManager &eManager) {
  // render skybox (render as last to prevent overdraw)

  glBindFramebuffer(GL_READ_FRAMEBUFFER, p_fboManager.getFBO("gBuffer"));
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, p_fboManager.getFBO("cubeFBO"));
  glBlitFramebuffer(0, 0, p_width, p_height, 0, 0, p_width, p_height, GL_DEPTH_BUFFER_BIT,
                    GL_NEAREST);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, p_fboManager.getFBO("lightFBO"));
  glBlitFramebuffer(0, 0, p_width, p_height, 0, 0, p_width, p_height, GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("cubeFBO"));

  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glDisable(GL_CULL_FACE);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  p_shaderProgram.use();
  eManager.getCamera().bindProjViewMatrix(p_shaderProgram.getUniformLocation("projection"),
                                          p_shaderProgram.getUniformLocation("view"));

  glActiveTexture(GL_TEXTURE0);
  p_textureManager.bindTexture("envCubemap");
  renderCube();
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_BLEND);
}

void CubeMapPass::setViewport(uint32_t w, uint32_t h) {
  p_width = w;
  p_height = h;

  glBindFramebuffer(GL_FRAMEBUFFER, p_fboManager.getFBO("cubeFBO"));

  uint32_t cubeFrame = p_textureManager.loadTexture("cubeFrame", GL_RGBA16F, GL_RGBA, GL_FLOAT,
                                                    p_width, p_height, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cubeFrame, 0);
  uint32_t cubeFrameBright;
  glGenTextures(1, &cubeFrameBright);
  glBindTexture(GL_TEXTURE_2D, cubeFrameBright);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, p_width, p_height, 0, GL_RGBA, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                  GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise
                                     // sample repeated texture values!
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, cubeFrameBright, 0);
  p_textureManager.setTexture("cubeFrameBright", cubeFrameBright, GL_TEXTURE_2D);

  uint32_t attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
  glDrawBuffers(2, attachments);
  glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, p_width, p_height);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeMapPass::generateCubeMap() {
  glGenTextures(1, &m_envCubemap);
  p_textureManager.setTexture("envCubemap", m_envCubemap, GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
  for (uint32_t i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 512, 512, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 512, 512);

  m_equirectangularToCubemapShader.use();
  m_equirectangularToCubemapShader.setUniformBinding("equirectangularMap");
  m_equirectangularToCubemapShader.setUniformBinding("projection");
  m_equirectangularToCubemapShader.setUniformBinding("view");

  glUniform1i(m_equirectangularToCubemapShader.getUniformLocation("equirectangularMap"), 0);
  glUniformMatrix4fv(m_equirectangularToCubemapShader.getUniformLocation("projection"), 1, GL_FALSE,
                     glm::value_ptr(m_captureProjection));

  glActiveTexture(GL_TEXTURE0);
  p_textureManager.bindTexture("hdrTexture");

  glViewport(0, 0, 512, 512); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  for (uint32_t i = 0; i < 6; ++i) {
    glUniformMatrix4fv(m_equirectangularToCubemapShader.getUniformLocation("view"), 1, GL_FALSE,
                       glm::value_ptr(m_captureViews[i]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           m_envCubemap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCube(); // renders a 1x1 cube
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void CubeMapPass::generateIrradianceMap() {
  uint32_t irradianceMap;
  glGenTextures(1, &irradianceMap);
  p_textureManager.setTexture("irradianceMap", irradianceMap, GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (uint32_t i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 32, 32, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 32, 32);

  m_irradianceShader.use();
  m_irradianceShader.setUniformBinding("environmentMap");
  m_irradianceShader.setUniformBinding("projection");
  m_irradianceShader.setUniformBinding("view");

  glUniform1i(m_irradianceShader.getUniformLocation("environmentMap"), 0);
  glUniformMatrix4fv(m_irradianceShader.getUniformLocation("projection"), 1, GL_FALSE,
                     glm::value_ptr(m_captureProjection));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

  glViewport(0, 0, 32, 32); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  for (uint32_t i = 0; i < 6; ++i) {
    glUniformMatrix4fv(m_irradianceShader.getUniformLocation("view"), 1, GL_FALSE,
                       glm::value_ptr(m_captureViews[i]));
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           irradianceMap, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeMapPass::generatePrefilterMap() {
  uint32_t prefilterMap;
  glGenTextures(1, &prefilterMap);
  p_textureManager.setTexture("prefilterMap", prefilterMap, GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (uint32_t i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, 128, 128, 0, GL_RGBA, GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification filter to mip_linear
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  m_prefilterShader.use();
  m_prefilterShader.setUniformBinding("environmentMap");
  m_prefilterShader.setUniformBinding("projection");
  m_prefilterShader.setUniformBinding("view");
  m_prefilterShader.setUniformBinding("roughness");

  glUniform1i(m_prefilterShader.getUniformLocation("environmentMap"), 0);
  glUniformMatrix4fv(m_prefilterShader.getUniformLocation("projection"), 1, GL_FALSE,
                     glm::value_ptr(m_captureProjection));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  uint32_t maxMipLevels = 5;
  for (uint32_t mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    uint32_t mipWidth = static_cast<uint32_t>(128 * std::pow(0.5, mip));
    uint32_t mipHeight = static_cast<uint32_t>(128 * std::pow(0.5, mip));
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    glUniform1f(m_prefilterShader.getUniformLocation("roughness"), roughness);
    for (uint32_t i = 0; i < 6; ++i) {
      glUniformMatrix4fv(m_prefilterShader.getUniformLocation("view"), 1, GL_FALSE,
                         glm::value_ptr(m_captureViews[i]));
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, prefilterMap, mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      renderCube();
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void CubeMapPass::generateBRDF() {
  uint32_t brdfLUTTexture;
  glGenTextures(1, &brdfLUTTexture);
  p_textureManager.setTexture("brdfLUT", brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

  glViewport(0, 0, 512, 512);
  m_brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  renderQuad();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void CubeMapPass::renderCube() {
  // initialize (if necessary)
  if (m_cubeVAO == 0) {
    // clang-format off
      float vertices[] = {
          // back face
          -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
           1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
           1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right
           1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
          -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
          -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
          // front face
          -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
           1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
           1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
           1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
          -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
          -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
          // left face
          -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
          -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
          -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
          -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
          -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
          -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
          // right face
           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
           1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right
           1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
           1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
           1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left
          // bottom face
          -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
           1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
           1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
           1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
          -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
          -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
          // top face
          -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
           1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
           1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right
           1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
          -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
          -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left
      };
    // clang-format on

    glGenVertexArrays(1, &m_cubeVAO);
    glGenBuffers(1, &m_cubeVBO);
    // fill buffer
    glBindBuffer(GL_ARRAY_BUFFER, m_cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    // link vertex attributes
    glBindVertexArray(m_cubeVAO);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
  }
  // render Cube
  glBindVertexArray(m_cubeVAO);
  glDrawArrays(GL_TRIANGLES, 0, 36);
  glBindVertexArray(0);
}

void CubeMapPass::renderQuad() {
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
  glBindVertexArray(0);
}
