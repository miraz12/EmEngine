#include "CubeMapPass.hpp"

#include <ECS/ECSManager.hpp>

#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "FrameGraph.hpp"
#include "RenderUtil.hpp"

CubeMapPass::CubeMapPass()
  : RenderPass("resources/Shaders/background.vert",
               "resources/Shaders/background.frag")
  , m_equirectangularToCubemapShader(
      "resources/Shaders/cubeMap.vert",
      "resources/Shaders/equirectangularToCubemap.frag")
  , m_irradianceShader("resources/Shaders/cubeMap.vert",
                       "resources/Shaders/irradiance.frag")
  , m_prefilterShader("resources/Shaders/cubeMap.vert",
                      "resources/Shaders/prefilter.frag")
  , m_brdfShader("resources/Shaders/quad.vert", "resources/Shaders/brdf.frag")
{

#if !defined(EMSCRIPTEN)
  glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#endif

  stbi_set_flip_vertically_on_load(true);
  i32 width;
  i32 height;
  i32 nrComponents;

  float* data = stbi_loadf("resources/Textures/clarens_midday_1k.hdr",
                           &width,
                           &height,
                           &nrComponents,
                           0);

  u32 hdrTexture = 0;
  if (data) {
    glGenTextures(1, &hdrTexture);
    glBindTexture(GL_TEXTURE_2D, hdrTexture);
    glTexImage2D(
      GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    m_textureManager.setTexture("hdrTexture", hdrTexture);

    stbi_image_free(data);
  } else {
    std::cout << "Failed to load HDR image.\n";
  }
  stbi_set_flip_vertically_on_load(false);

  glGenFramebuffers(1, &m_captureFBO);
  m_fboManager.setFBO("iblCaptureFBO", m_captureFBO);
  glGenRenderbuffers(1, &m_captureRBO);
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_captureRBO);

  // Only generate IBL maps if HDR texture loaded successfully
  if (hdrTexture != 0) {
    generateCubeMap();
    generateIrradianceMap();
    generatePrefilterMap();
  }
  generateBRDF();

  m_shaderProgram.setUniformBinding("projection");
  m_shaderProgram.setUniformBinding("view");
  m_shaderProgram.setUniformBinding("FragColor");
  m_shaderProgram.setUniformBinding("FragColorBright");

  glGenFramebuffers(1, &m_cubeBuffer);
  glGenRenderbuffers(1, &m_rbo);
  m_fboManager.setFBO("cubeFBO", m_cubeBuffer);
  glViewport(0, 0, m_width, m_height);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void
CubeMapPass::Init(FrameGraph& fGraph)
{
  fGraph.getPass(PassId::kLight)->addTexture("irradianceMap");
  fGraph.getPass(PassId::kLight)->addTexture("prefilterMap");
  fGraph.getPass(PassId::kLight)->addTexture("brdfLUT");
}

void
CubeMapPass::Execute(ECSManager& eManager)
{
#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPushDebugGroup(GL_DEBUG_SOURCE_APPLICATION, 0, -1, "Background Pass");
#endif
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboManager.getFBO("gBuffer"));
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fboManager.getFBO("cubeFBO"));
  glBlitFramebuffer(0,
                    0,
                    m_width,
                    m_height,
                    0,
                    0,
                    m_width,
                    m_height,
                    GL_DEPTH_BUFFER_BIT,
                    GL_NEAREST);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fboManager.getFBO("lightFBO"));
  glBlitFramebuffer(0,
                    0,
                    m_width,
                    m_height,
                    0,
                    0,
                    m_width,
                    m_height,
                    GL_COLOR_BUFFER_BIT,
                    GL_NEAREST);
  glBindFramebuffer(GL_FRAMEBUFFER, m_fboManager.getFBO("cubeFBO"));

  glEnable(GL_BLEND);
  glBlendFuncSeparate(
    GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
  glBlendEquation(GL_FUNC_ADD);
  glDisable(GL_CULL_FACE);

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  m_shaderProgram.use();
  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  CameraSystem::bindProjViewMatrix(
    cam,
    m_shaderProgram.getUniformLocation("projection"),
    m_shaderProgram.getUniformLocation("view"));

  glActiveTexture(GL_TEXTURE0);
  m_textureManager.bindTexture("envCubemap");
  Util::renderCube();

  // Clean up GL state
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glDisable(GL_BLEND);
  glDepthFunc(GL_LESS);
  glEnable(GL_CULL_FACE);
  glCullFace(GL_BACK);

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  glPopDebugGroup();
#endif
}

void
CubeMapPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  glBindFramebuffer(GL_FRAMEBUFFER, m_fboManager.getFBO("cubeFBO"));

  u32 cubeFrame = m_textureManager.loadTexture(
    "cubeFrame", GL_RGBA16F, GL_RGBA, GL_FLOAT, m_width, m_height, 0);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, cubeFrame, 0);

  u32 attachments[1] = { GL_COLOR_ATTACHMENT0 };
  glDrawBuffers(1, attachments);
  glBindRenderbuffer(GL_RENDERBUFFER, m_rbo);
  glRenderbufferStorage(
    GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, m_width, m_height);
  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_rbo);
  // finally check if framebuffer is complete
  if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
    std::cout << "Framebuffer not complete!" << std::endl;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
CubeMapPass::generateCubeMap()
{
  glGenTextures(1, &m_envCubemap);
  m_textureManager.setTexture("envCubemap", m_envCubemap, GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
  for (u32 i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0,
                 GL_RGBA16F,
                 512,
                 512,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(
    GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, 512, 512);

  m_equirectangularToCubemapShader.use();
  m_equirectangularToCubemapShader.setUniformBinding("equirectangularMap");
  m_equirectangularToCubemapShader.setUniformBinding("projection");
  m_equirectangularToCubemapShader.setUniformBinding("view");

  glUniform1i(
    m_equirectangularToCubemapShader.getUniformLocation("equirectangularMap"),
    0);
  glUniformMatrix4fv(
    m_equirectangularToCubemapShader.getUniformLocation("projection"),
    1,
    GL_FALSE,
    glm::value_ptr(m_captureProjection));

  glActiveTexture(GL_TEXTURE0);
  m_textureManager.bindTexture("hdrTexture");

  glViewport(
    0,
    0,
    512,
    512); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  for (u32 i = 0; i < 6; ++i) {
    glUniformMatrix4fv(
      m_equirectangularToCubemapShader.getUniformLocation("view"),
      1,
      GL_FALSE,
      glm::value_ptr(m_captureViews[i]));
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           m_envCubemap,
                           0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Util::renderCube(); // renders a 1x1 cube
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
}

void
CubeMapPass::generateIrradianceMap()
{
  u32 irradianceMap;
  glGenTextures(1, &irradianceMap);
  m_textureManager.setTexture(
    "irradianceMap", irradianceMap, GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, irradianceMap);
  for (u32 i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0,
                 GL_RGBA16F,
                 32,
                 32,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
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
  glUniformMatrix4fv(m_irradianceShader.getUniformLocation("projection"),
                     1,
                     GL_FALSE,
                     glm::value_ptr(m_captureProjection));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

  glViewport(
    0,
    0,
    32,
    32); // don't forget to configure the viewport to the capture dimensions.
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  for (u32 i = 0; i < 6; ++i) {
    glUniformMatrix4fv(m_irradianceShader.getUniformLocation("view"),
                       1,
                       GL_FALSE,
                       glm::value_ptr(m_captureViews[i]));
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                           irradianceMap,
                           0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Util::renderCube();
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void
CubeMapPass::generatePrefilterMap()
{
  u32 prefilterMap;
  glGenTextures(1, &prefilterMap);
  m_textureManager.setTexture(
    "prefilterMap", prefilterMap, GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, prefilterMap);
  for (u32 i = 0; i < 6; ++i) {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                 0,
                 GL_RGBA16F,
                 128,
                 128,
                 0,
                 GL_RGBA,
                 GL_FLOAT,
                 nullptr);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP,
                  GL_TEXTURE_MIN_FILTER,
                  GL_LINEAR_MIPMAP_LINEAR); // be sure to set minification
                                            // filter to mip_linear
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // generate mipmaps for the cubemap so OpenGL automatically allocates the
  // required memory.
  glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

  m_prefilterShader.use();
  m_prefilterShader.setUniformBinding("environmentMap");
  m_prefilterShader.setUniformBinding("projection");
  m_prefilterShader.setUniformBinding("view");
  m_prefilterShader.setUniformBinding("roughness");

  glUniform1i(m_prefilterShader.getUniformLocation("environmentMap"), 0);
  glUniformMatrix4fv(m_prefilterShader.getUniformLocation("projection"),
                     1,
                     GL_FALSE,
                     glm::value_ptr(m_captureProjection));

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_envCubemap);

  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  u32 maxMipLevels = 5;
  for (u32 mip = 0; mip < maxMipLevels; ++mip) {
    // reisze framebuffer according to mip-level size.
    u32 mipWidth = static_cast<u32>(128 * std::pow(0.5, mip));
    u32 mipHeight = static_cast<u32>(128 * std::pow(0.5, mip));
    glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
    glRenderbufferStorage(
      GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, mipWidth, mipHeight);
    glViewport(0, 0, mipWidth, mipHeight);

    float roughness = (float)mip / (float)(maxMipLevels - 1);
    glUniform1f(m_prefilterShader.getUniformLocation("roughness"), roughness);
    for (u32 i = 0; i < 6; ++i) {
      glUniformMatrix4fv(m_prefilterShader.getUniformLocation("view"),
                         1,
                         GL_FALSE,
                         glm::value_ptr(m_captureViews[i]));
      glFramebufferTexture2D(GL_FRAMEBUFFER,
                             GL_COLOR_ATTACHMENT0,
                             GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                             prefilterMap,
                             mip);

      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
      Util::renderCube();
    }
  }
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void
CubeMapPass::generateBRDF()
{
  u32 brdfLUTTexture;
  glGenTextures(1, &brdfLUTTexture);
  m_textureManager.setTexture("brdfLUT", brdfLUTTexture);

  // pre-allocate enough memory for the LUT texture.
  glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, 0);
  // be sure to set wrapping mode to GL_CLAMP_TO_EDGE
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // then re-configure capture framebuffer object and render screen-space quad
  // with BRDF shader.
  glBindFramebuffer(GL_FRAMEBUFFER, m_captureFBO);
  glBindRenderbuffer(GL_RENDERBUFFER, m_captureRBO);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, 512, 512);
  glFramebufferTexture2D(
    GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, brdfLUTTexture, 0);

  glViewport(0, 0, 512, 512);
  m_brdfShader.use();
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  Util::renderQuad();

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
