#include "CubeMapPass.hpp"

#include <ECS/ECSManager.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/UBOStructs.hpp>
#include <array>

#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "FrameGraph.hpp"
#include "RenderUtil.hpp"

CubeMapPass::CubeMapPass()
  : RenderPass("CubeMapPass",
               "resources/Shaders/background.vert",
               "resources/Shaders/background.frag")
{
  auto& resources = gfx::RenderResources::getInstance();

  // Load additional shaders for IBL generation
  resources.loadShaderProgram(
    m_equirectToCubeName,
    "resources/Shaders/cubeMap.vert",
    "resources/Shaders/equirectangularToCubemap.frag");
  resources.loadShaderProgram(m_irradianceName,
                              "resources/Shaders/cubeMap.vert",
                              "resources/Shaders/irradiance.frag");
  resources.loadShaderProgram(m_prefilterName,
                              "resources/Shaders/cubeMap.vert",
                              "resources/Shaders/prefilter.frag");
  resources.loadShaderProgram(
    m_brdfName, "resources/Shaders/quad.vert", "resources/Shaders/brdf.frag");

  resources.setSeamlessCubemap(true);

  stbi_set_flip_vertically_on_load(true);
  i32 width;
  i32 height;
  i32 nrComponents;

  float* data = stbi_loadf("resources/Textures/clarens_midday_1k.hdr",
                           &width,
                           &height,
                           &nrComponents,
                           0);

  bool hdrLoaded = false;
  if (data) {
    gfx::TextureCreateInfo hdrInfo{};
    hdrInfo.width = static_cast<u32>(width);
    hdrInfo.height = static_cast<u32>(height);
    hdrInfo.format = gfx::PixelFormat::RGB32F;
    hdrInfo.mipLevels = 1;
    hdrInfo.initialData = data;
    hdrInfo.debugName = "hdrTexture";
    resources.createTexture2D("hdrTexture", hdrInfo);
    hdrLoaded = true;

    stbi_image_free(data);
  } else {
    std::cout << "Failed to load HDR image.\n";
  }
  stbi_set_flip_vertically_on_load(false);

  // Create FBO through new RenderResources system (bare FBO for IBL capture)
  resources.createBareFramebuffer("iblCaptureFBO");

  // Create capture RBO through abstraction (initial size 512x512 for env
  // cubemap)
  gfx::RenderbufferCreateInfo captureRboInfo{};
  captureRboInfo.width = 512;
  captureRboInfo.height = 512;
  captureRboInfo.format = gfx::PixelFormat::Depth16;
  captureRboInfo.debugName = "iblCaptureRBO";
  resources.createRenderbuffer("iblCaptureRBO", captureRboInfo);

  // Attach RBO to FBO
  resources.setFramebufferRenderbuffer(
    "iblCaptureFBO", gfx::RenderbufferAttachment::Depth, "iblCaptureRBO");

  // Only generate IBL maps if HDR texture loaded successfully
  if (hdrLoaded) {
    generateCubeMap();
    generateIrradianceMap();
    generatePrefilterMap();
  }
  generateBRDF();

  // Bind CameraData uniform block for background shader
  useShader();
  resources.bindShaderUniformBlock(
    getShaderId(), "CameraData", gfx::UBOBinding::Camera);

  // Create pipeline for background cube rendering
  // Cube layout: vec3 position, vec3 normal, vec2 texcoord (8 floats, 32 bytes
  // stride)
  static constexpr u32 kCubeStride = 8 * sizeof(float);
  std::array<gfx::VertexBinding, 1> cubeBindings = {
    { { .binding = 0, .stride = kCubeStride, .perInstance = false } }
  };
  std::array<gfx::VertexAttribute, 3> cubeAttributes = { {
    { .location = 0,
      .binding = 0,
      .offset = 0,
      .format = gfx::PixelFormat::RGB32F }, // position
    { .location = 1,
      .binding = 0,
      .offset = 3 * sizeof(float),
      .format = gfx::PixelFormat::RGB32F }, // normal
    { .location = 2,
      .binding = 0,
      .offset = 6 * sizeof(float),
      .format = gfx::PixelFormat::RG32F } // texcoord
  } };

  gfx::PipelineCreateInfo bgPipeInfo{};
  bgPipeInfo.shaderProgram = getShaderId();
  bgPipeInfo.vertexBindings = cubeBindings;
  bgPipeInfo.vertexAttributes = cubeAttributes;
  bgPipeInfo.topology = gfx::PrimitiveTopology::Triangles;
  bgPipeInfo.depthStencil.depthTestEnable = true;
  bgPipeInfo.depthStencil.depthWriteEnable = false;
  bgPipeInfo.depthStencil.depthCompareOp = gfx::CompareOp::LessEqual;
  bgPipeInfo.blend.attachments[0].blendEnable = true;
  bgPipeInfo.blend.attachments[0].srcColorBlendFactor =
    gfx::BlendFactor::SrcAlpha;
  bgPipeInfo.blend.attachments[0].dstColorBlendFactor =
    gfx::BlendFactor::OneMinusSrcAlpha;
  bgPipeInfo.blend.attachments[0].colorBlendOp = gfx::BlendOp::Add;
  bgPipeInfo.blend.attachments[0].srcAlphaBlendFactor = gfx::BlendFactor::One;
  bgPipeInfo.blend.attachments[0].dstAlphaBlendFactor =
    gfx::BlendFactor::OneMinusSrcAlpha;
  bgPipeInfo.blend.attachments[0].alphaBlendOp = gfx::BlendOp::Add;
  bgPipeInfo.rasterizer.cullMode = gfx::CullMode::None;
  bgPipeInfo.debugName = "BackgroundCubePipeline";
  m_backgroundPipeline =
    resources.createPipeline("BackgroundCubePipeline", bgPipeInfo);

  // Create FBO through new RenderResources system (bare FBO, attachments
  // managed in setViewport)
  resources.createBareFramebuffer("cubeFBO");

  // Create renderbuffer for depth through new RenderResources system
  gfx::RenderbufferCreateInfo rboInfo{};
  rboInfo.width = m_width;
  rboInfo.height = m_height;
  rboInfo.format = gfx::PixelFormat::Depth24Stencil8;
  rboInfo.debugName = "cubeFBODepth";
  resources.createRenderbuffer("cubeFBODepth", rboInfo);

  // Create cubeFrame texture through new RenderResources system
  // This is read by BloomPass
  gfx::TextureCreateInfo cubeFrameInfo{};
  cubeFrameInfo.width = m_width;
  cubeFrameInfo.height = m_height;
  cubeFrameInfo.format = gfx::PixelFormat::RGBA16F;
  cubeFrameInfo.mipLevels = 1;
  cubeFrameInfo.debugName = "cubeFrame";
  resources.createTexture2D("cubeFrame", cubeFrameInfo);
  m_useNewResources = true;

  setViewport(m_width, m_height);
  resources.bindDefaultFramebuffer();
}

void
CubeMapPass::Init(FrameGraph& fGraph)
{
  fGraph.getPass(PassId::kLight)->addTexture("irradianceMap");
  fGraph.getPass(PassId::kLight)->addTexture("prefilterMap");
  fGraph.getPass(PassId::kLight)->addTexture("brdfLUT");
}

void
CubeMapPass::Record(ECSManager& /* eManager */)
{
  auto& resources = gfx::RenderResources::getInstance();

  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->pushDebugGroup("Background Pass");
#endif

  // Blit depth from gBuffer to cubeFBO
  auto gBufferFBO = resources.getFramebuffer("gBuffer");
  auto cubeFBO = resources.getFramebuffer("cubeFBO");
  auto w = static_cast<i32>(m_width);
  auto h = static_cast<i32>(m_height);
  cmd->blitFramebuffer(
    gBufferFBO, cubeFBO, 0, 0, w, h, 0, 0, w, h, false, true, false, false);

  // Blit color from lightFBO to cubeFBO
  auto lightFBO = resources.getFramebuffer("lightFBO");
  cmd->blitFramebuffer(
    lightFBO, cubeFBO, 0, 0, w, h, 0, 0, w, h, true, false, false, false);

  // Begin render pass (no clear - we want to keep the blitted content)
  gfx::RenderPassBeginInfo passInfo{};
  passInfo.framebuffer = resources.getFramebuffer("cubeFBO");
  passInfo.colorAttachmentCount = 1;
  passInfo.clearColor = false;
  passInfo.clearDepthStencil = false;
  cmd->beginRenderPass(passInfo);

  // Bind pipeline (includes depth test, blend, cull mode)
  cmd->bindPipeline(m_backgroundPipeline);

  // Set viewport
  gfx::Viewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(m_width);
  viewport.height = static_cast<float>(m_height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  cmd->setViewport(viewport);

  // Bind cube vertex buffer
  cmd->bindVertexBuffer(0, resources.getCubeVertexBuffer(), 0);

  // Bind environment cubemap texture
  gfx::SamplerId linearClampSampler = resources.getLinearClampSampler();
  cmd->bindTexture(0, resources.getTexture("envCubemap"), linearClampSampler);

  // Draw cube
  cmd->draw(gfx::RenderResources::kCubeVertexCount, 1, 0, 0);

  cmd->endRenderPass();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif
}

void
CubeMapPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  auto& resources = gfx::RenderResources::getInstance();

  // Recreate cubeFrame texture and renderbuffer with new size
  if (m_useNewResources) {
    resources.recreateTexture2D("cubeFrame", m_width, m_height);
    resources.resizeRenderbuffer("cubeFBODepth", m_width, m_height);
  }

  // Attach cubeFrame texture to FBO
  resources.setFramebufferAttachment("cubeFBO", 0, "cubeFrame");

  // Set draw buffers
  std::array<u32, 1> drawBuffers = { 0 };
  resources.setDrawBuffers("cubeFBO", drawBuffers);

  // Attach depth renderbuffer
  resources.setFramebufferRenderbuffer(
    "cubeFBO", gfx::RenderbufferAttachment::Depth, "cubeFBODepth");

  // Check framebuffer completeness
  if (!resources.isFramebufferComplete("cubeFBO")) {
    std::cout << "Framebuffer not complete!\n";
  }
}

void
CubeMapPass::generateCubeMap()
{
  auto& resources = gfx::RenderResources::getInstance();

  // Create 512x512 env cubemap with mipmaps through abstraction
  gfx::TextureCreateInfo envInfo{};
  envInfo.type = gfx::TextureType::TextureCube;
  envInfo.width = 512;
  envInfo.height = 512;
  envInfo.format = gfx::PixelFormat::RGBA16F;
  envInfo.mipLevels = 5; // Enough for IBL prefiltering
  envInfo.debugName = "envCubemap";
  resources.createTextureCube("envCubemap", envInfo);

  // Resize capture RBO for 512x512
  resources.resizeRenderbuffer("iblCaptureRBO", 512, 512);

  auto& device = gfx::GraphicsDevice::getInstance();
  gfx::ShaderId equirectShader =
    resources.getShaderProgram(m_equirectToCubeName);
  resources.bindShaderProgram(m_equirectToCubeName);

  device.setUniformInt(
    device.getUniformLocation(equirectShader, "equirectangularMap"), 0);
  device.setUniformMat4(device.getUniformLocation(equirectShader, "projection"),
                        m_captureProjection);

  resources.bindTexture(0, "hdrTexture");

  resources.setViewportRect(0, 0, 512, 512);
  resources.bindFramebuffer("iblCaptureFBO");
  for (u32 face = 0; face < 6; ++face) {
    device.setUniformMat4(device.getUniformLocation(equirectShader, "view"),
                          m_captureViews[face]);
    // Attach cubemap face using abstraction
    resources.setFramebufferAttachment(
      "iblCaptureFBO", 0, "envCubemap", 0, face);

    resources.clearColor(std::nullopt, glm::vec4(0.0f));
    resources.clearDepth(1.0f);

    Util::renderCube();
  }

  resources.bindDefaultFramebuffer();

  // Generate mipmaps through abstraction
  gfx::GraphicsDevice::getInstance().generateMipmaps(
    resources.getTexture("envCubemap"));
}

void
CubeMapPass::generateIrradianceMap()
{
  auto& resources = gfx::RenderResources::getInstance();

  // Create 32x32 irradiance cubemap through abstraction (no mipmaps needed)
  gfx::TextureCreateInfo irradianceInfo{};
  irradianceInfo.type = gfx::TextureType::TextureCube;
  irradianceInfo.width = 32;
  irradianceInfo.height = 32;
  irradianceInfo.format = gfx::PixelFormat::RGBA16F;
  irradianceInfo.mipLevels = 1;
  irradianceInfo.debugName = "irradianceMap";
  resources.createTextureCube("irradianceMap", irradianceInfo);

  // Resize capture RBO for 32x32
  resources.resizeRenderbuffer("iblCaptureRBO", 32, 32);

  auto& device = gfx::GraphicsDevice::getInstance();
  gfx::ShaderId irradianceShader = resources.getShaderProgram(m_irradianceName);
  resources.bindShaderProgram(m_irradianceName);

  device.setUniformInt(
    device.getUniformLocation(irradianceShader, "environmentMap"), 0);
  device.setUniformMat4(
    device.getUniformLocation(irradianceShader, "projection"),
    m_captureProjection);

  resources.bindTexture(0, "envCubemap");

  resources.setViewportRect(0, 0, 32, 32);
  resources.bindFramebuffer("iblCaptureFBO");
  for (u32 face = 0; face < 6; ++face) {
    device.setUniformMat4(device.getUniformLocation(irradianceShader, "view"),
                          m_captureViews[face]);
    // Attach cubemap face using abstraction
    resources.setFramebufferAttachment(
      "iblCaptureFBO", 0, "irradianceMap", 0, face);
    resources.clearColor(std::nullopt, glm::vec4(0.0f));
    resources.clearDepth(1.0f);

    Util::renderCube();
  }
  resources.bindDefaultFramebuffer();
}

void
CubeMapPass::generatePrefilterMap()
{
  auto& resources = gfx::RenderResources::getInstance();

  // Create 128x128 prefilter cubemap with 5 mip levels through abstraction
  constexpr u32 kMaxMipLevels = 5;
  gfx::TextureCreateInfo prefilterInfo{};
  prefilterInfo.type = gfx::TextureType::TextureCube;
  prefilterInfo.width = 128;
  prefilterInfo.height = 128;
  prefilterInfo.format = gfx::PixelFormat::RGBA16F;
  prefilterInfo.mipLevels = kMaxMipLevels;
  prefilterInfo.debugName = "prefilterMap";
  resources.createTextureCube("prefilterMap", prefilterInfo);

  auto& device = gfx::GraphicsDevice::getInstance();
  gfx::ShaderId prefilterShader = resources.getShaderProgram(m_prefilterName);
  resources.bindShaderProgram(m_prefilterName);

  device.setUniformInt(
    device.getUniformLocation(prefilterShader, "environmentMap"), 0);
  device.setUniformMat4(
    device.getUniformLocation(prefilterShader, "projection"),
    m_captureProjection);

  resources.bindTexture(0, "envCubemap");

  resources.bindFramebuffer("iblCaptureFBO");
  for (u32 mip = 0; mip < kMaxMipLevels; ++mip) {
    // Resize RBO according to mip-level size
    u32 mipSize = 128 >> mip;
    resources.resizeRenderbuffer("iblCaptureRBO", mipSize, mipSize);
    resources.setViewportRect(0, 0, mipSize, mipSize);

    float roughness =
      static_cast<float>(mip) / static_cast<float>(kMaxMipLevels - 1);
    device.setUniformFloat(
      device.getUniformLocation(prefilterShader, "roughness"), roughness);

    for (u32 face = 0; face < 6; ++face) {
      device.setUniformMat4(device.getUniformLocation(prefilterShader, "view"),
                            m_captureViews[face]);
      // Attach cubemap face at specific mip level using abstraction
      resources.setFramebufferAttachment(
        "iblCaptureFBO", 0, "prefilterMap", mip, face);

      resources.clearColor(std::nullopt, glm::vec4(0.0f));
      resources.clearDepth(1.0f);
      Util::renderCube();
    }
  }
  resources.bindDefaultFramebuffer();
}
void
CubeMapPass::generateBRDF()
{
  auto& resources = gfx::RenderResources::getInstance();

  // Create BRDF LUT texture through abstraction
  gfx::TextureCreateInfo brdfInfo{};
  brdfInfo.width = 512;
  brdfInfo.height = 512;
  brdfInfo.format = gfx::PixelFormat::RG16F;
  brdfInfo.mipLevels = 1;
  brdfInfo.debugName = "brdfLUT";
  resources.createTexture2D("brdfLUT", brdfInfo);

  // Resize capture RBO and attach BRDF texture
  resources.resizeRenderbuffer("iblCaptureRBO", 512, 512);
  resources.bindFramebuffer("iblCaptureFBO");
  resources.setFramebufferAttachment("iblCaptureFBO", 0, "brdfLUT", 0, 0);

  resources.setViewportRect(0, 0, 512, 512);
  resources.bindShaderProgram(m_brdfName);
  resources.clearColor(std::nullopt, glm::vec4(0.0f));
  resources.clearDepth(1.0f);
  Util::renderQuad();

  resources.bindDefaultFramebuffer();
}
