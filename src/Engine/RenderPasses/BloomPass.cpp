#include "BloomPass.hpp"
#include "RenderUtil.hpp"
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/UBOStructs.hpp>
#include <array>
#include <iostream>

BloomPass::BloomPass()
  : RenderPass("BloomUp",
               "resources/Shaders/quad.vert",
               "resources/Shaders/bloomUp.frag")
{
  auto& resources = gfx::RenderResources::getInstance();

  // Load additional shaders
  resources.loadShaderProgram(m_extractBrightName,
                              "resources/Shaders/quad.vert",
                              "resources/Shaders/extractBright.frag");
  resources.loadShaderProgram(m_downShaderName,
                              "resources/Shaders/quad.vert",
                              "resources/Shaders/bloomDown.frag");
  resources.loadShaderProgram(m_bloomCombineName,
                              "resources/Shaders/quad.vert",
                              "resources/Shaders/bloomCombine.frag");

  // Create FBOs through new RenderResources system (bare FBOs, attachments
  // managed separately)
  resources.createBareFramebuffer("brightFBO");
  resources.createBareFramebuffer("bloomFBO");
  resources.createBareFramebuffer("bloomFinalFBO");

  // Create main textures through new RenderResources system
  // These are used by other passes (FxaaPass reads frameBloomFinal)
  gfx::TextureCreateInfo brightInfo{};
  brightInfo.width = m_width;
  brightInfo.height = m_height;
  brightInfo.format = gfx::PixelFormat::RGBA16F;
  brightInfo.mipLevels = 1;
  brightInfo.debugName = "frameBright";
  resources.createTexture2D("frameBright", brightInfo);

  gfx::TextureCreateInfo finalInfo{};
  finalInfo.width = m_width;
  finalInfo.height = m_height;
  finalInfo.format = gfx::PixelFormat::RGBA16F;
  finalInfo.mipLevels = 1;
  finalInfo.debugName = "frameBloomFinal";
  resources.createTexture2D("frameBloomFinal", finalInfo);

  m_useNewResources = true;

  // Create mip chain textures through RenderResources
  glm::vec2 currentMipSize(m_width, m_height);
  glm::ivec2 currentMipSizeInt(m_width, m_height);
  for (u32 i = 0; i < 5; i++) {
    mipLevel mip;
    currentMipSize *= 0.5f;
    currentMipSizeInt /= 2;
    mip.size = currentMipSize;
    mip.intSize = currentMipSizeInt;
    mip.textureName = "bloomMip" + std::to_string(i);

    gfx::TextureCreateInfo mipInfo{};
    mipInfo.width = static_cast<u32>(currentMipSize.x);
    mipInfo.height = static_cast<u32>(currentMipSize.y);
    mipInfo.format = gfx::PixelFormat::R11G11B10F;
    mipInfo.mipLevels = 1;
    mipInfo.debugName = mip.textureName.c_str();
    resources.createTexture2D(mip.textureName, mipInfo);

    m_mipChain.emplace_back(mip);
  }

  setViewport(m_width, m_height);

  // Bind PostProcessData UBO block for all bloom shaders
  useShader();
  resources.bindShaderUniformBlock(
    getShaderId(), "PostProcessData", gfx::UBOBinding::PostProcess);

  gfx::ShaderId downShader = resources.getShaderProgram(m_downShaderName);
  resources.bindShaderProgram(m_downShaderName);
  resources.bindShaderUniformBlock(
    downShader, "PostProcessData", gfx::UBOBinding::PostProcess);

  gfx::ShaderId combineShader = resources.getShaderProgram(m_bloomCombineName);
  resources.bindShaderProgram(m_bloomCombineName);
  resources.bindShaderUniformBlock(
    combineShader, "PostProcessData", gfx::UBOBinding::PostProcess);

  // Cache sampler uniform locations (samplers must remain regular uniforms)
  auto& device = gfx::GraphicsDevice::getInstance();
  m_combineSceneLoc = device.getUniformLocation(combineShader, "scene");
  m_combineBloomBlurLoc = device.getUniformLocation(combineShader, "bloomBlur");

  // Create pipelines for CommandBuffer rendering
  // All bloom passes use the same quad vertex layout: vec3 position, vec2
  // texcoord
  static constexpr u32 kQuadStride = 5 * sizeof(float);
  std::array<gfx::VertexBinding, 1> bindings = {
    { { .binding = 0, .stride = kQuadStride, .perInstance = false } }
  };
  std::array<gfx::VertexAttribute, 2> attributes = {
    { { .location = 0,
        .binding = 0,
        .offset = 0,
        .format = gfx::PixelFormat::RGB32F },
      { .location = 1,
        .binding = 0,
        .offset = 3 * sizeof(float),
        .format = gfx::PixelFormat::RG32F } }
  };

  // Common pipeline settings for fullscreen quad rendering
  auto createBloomPipeline = [&](const std::string& name,
                                 gfx::ShaderId shader,
                                 bool enableBlend = false) -> gfx::PipelineId {
    gfx::PipelineCreateInfo pipeInfo{};
    pipeInfo.shaderProgram = shader;
    pipeInfo.vertexBindings = bindings;
    pipeInfo.vertexAttributes = attributes;
    pipeInfo.topology = gfx::PrimitiveTopology::TriangleStrip;
    pipeInfo.depthStencil.depthTestEnable = false;
    pipeInfo.depthStencil.depthWriteEnable = false;
    pipeInfo.rasterizer.cullMode = gfx::CullMode::None;
    pipeInfo.debugName = name.c_str();
    if (enableBlend) {
      pipeInfo.blend.attachments[0].blendEnable = true;
      pipeInfo.blend.attachments[0].srcColorBlendFactor = gfx::BlendFactor::One;
      pipeInfo.blend.attachments[0].dstColorBlendFactor = gfx::BlendFactor::One;
      pipeInfo.blend.attachments[0].colorBlendOp = gfx::BlendOp::Add;
      pipeInfo.blend.attachments[0].srcAlphaBlendFactor = gfx::BlendFactor::One;
      pipeInfo.blend.attachments[0].dstAlphaBlendFactor = gfx::BlendFactor::One;
      pipeInfo.blend.attachments[0].alphaBlendOp = gfx::BlendOp::Add;
    } else {
      pipeInfo.blend.attachments[0].blendEnable = false;
    }
    return resources.createPipeline(name, pipeInfo);
  };

  gfx::ShaderId extractShader = resources.getShaderProgram(m_extractBrightName);
  m_extractBrightPipeline =
    createBloomPipeline("ExtractBrightPipeline", extractShader, false);
  m_downPipeline = createBloomPipeline("BloomDownPipeline", downShader, false);
  m_upPipeline = createBloomPipeline(
    "BloomUpPipeline", getShaderId(), true); // Additive blend
  m_combinePipeline =
    createBloomPipeline("BloomCombinePipeline", combineShader, false);

  resources.bindDefaultFramebuffer();
}

void
BloomPass::Execute(ECSManager& /* eManager */)
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();
  gfx::PostProcessUBO& postProcessUBO = resources.getPostProcessUBO();

  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

  gfx::SamplerId linearClampSampler = resources.getLinearClampSampler();
  gfx::FramebufferId brightFBO = resources.getFramebuffer("brightFBO");
  gfx::FramebufferId bloomFBO = resources.getFramebuffer("bloomFBO");
  gfx::FramebufferId bloomFinalFBO = resources.getFramebuffer("bloomFinalFBO");

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->pushDebugGroup("Bloom Pass");
#endif

  // =========================================================================
  // Stage 1: Extract bright pixels
  // =========================================================================
  gfx::RenderPassBeginInfo extractPassInfo{};
  extractPassInfo.framebuffer = brightFBO;
  extractPassInfo.colorAttachmentCount = 1;
  extractPassInfo.clearColor = false;
  extractPassInfo.clearDepthStencil = false;
  cmd->beginRenderPass(extractPassInfo);

  cmd->bindPipeline(m_extractBrightPipeline);

  gfx::Viewport fullViewport{};
  fullViewport.x = 0;
  fullViewport.y = 0;
  fullViewport.width = static_cast<float>(m_width);
  fullViewport.height = static_cast<float>(m_height);
  fullViewport.minDepth = 0.0f;
  fullViewport.maxDepth = 1.0f;
  cmd->setViewport(fullViewport);

  cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);
  cmd->bindTexture(0, resources.getTexture("cubeFrame"), linearClampSampler);
  cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);

  cmd->endRenderPass();

  // Submit extract pass before downsample since we need to set UBO for
  // downsample
  device.submit(m_cmdBuffer);

  // =========================================================================
  // Stage 2: Progressive downsample through mip chain
  // =========================================================================
  // Set initial resolution and mipLevel=0 for Karis average
  postProcessUBO.resolution = glm::vec4(
    static_cast<float>(m_width), static_cast<float>(m_height), 1.0f, 0.005f);
  postProcessUBO.postConfig = glm::ivec4(0, 0, 0, 0); // mipLevel = 0
  resources.flushPostProcessUBO();

  gfx::TextureId prevTexture = resources.getTexture("frameBright");

  for (u32 i = 0; i < static_cast<u32>(m_mipChain.size()); i++) {
    const mipLevel& mip = m_mipChain[i];

    // Reset command buffer for this mip level
    cmd->reset();

    // Set framebuffer attachment for this mip level
    cmd->setFramebufferAttachment(
      bloomFBO, 0, resources.getTexture(mip.textureName), 0, 0);

    gfx::RenderPassBeginInfo downPassInfo{};
    downPassInfo.framebuffer = bloomFBO;
    downPassInfo.colorAttachmentCount = 1;
    downPassInfo.clearColor = false;
    downPassInfo.clearDepthStencil = false;
    cmd->beginRenderPass(downPassInfo);

    cmd->bindPipeline(m_downPipeline);

    gfx::Viewport mipViewport{};
    mipViewport.x = 0;
    mipViewport.y = 0;
    mipViewport.width = mip.size.x;
    mipViewport.height = mip.size.y;
    mipViewport.minDepth = 0.0f;
    mipViewport.maxDepth = 1.0f;
    cmd->setViewport(mipViewport);

    cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);
    cmd->bindTexture(0, prevTexture, linearClampSampler);
    cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);

    cmd->endRenderPass();

    device.submit(m_cmdBuffer);

    // Update UBO for next iteration
    postProcessUBO.resolution.x = mip.size.x;
    postProcessUBO.resolution.y = mip.size.y;
    if (i == 0) {
      postProcessUBO.postConfig.x = 1; // mipLevel = 1 (disable Karis average)
    }
    resources.flushPostProcessUBO();

    prevTexture = resources.getTexture(mip.textureName);
  }

  // =========================================================================
  // Stage 3: Progressive upsample through mip chain (additive blending)
  // =========================================================================
  for (u32 i = static_cast<u32>(m_mipChain.size()) - 1; i > 0; i--) {
    const mipLevel& mip = m_mipChain[i];
    const mipLevel& nextMip = m_mipChain[i - 1];

    cmd->reset();

    // Set framebuffer attachment for next mip level
    cmd->setFramebufferAttachment(
      bloomFBO, 0, resources.getTexture(nextMip.textureName), 0, 0);

    gfx::RenderPassBeginInfo upPassInfo{};
    upPassInfo.framebuffer = bloomFBO;
    upPassInfo.colorAttachmentCount = 1;
    upPassInfo.clearColor = false;
    upPassInfo.clearDepthStencil = false;
    cmd->beginRenderPass(upPassInfo);

    cmd->bindPipeline(m_upPipeline); // Uses additive blend

    gfx::Viewport upViewport{};
    upViewport.x = 0;
    upViewport.y = 0;
    upViewport.width = nextMip.size.x;
    upViewport.height = nextMip.size.y;
    upViewport.minDepth = 0.0f;
    upViewport.maxDepth = 1.0f;
    cmd->setViewport(upViewport);

    cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);
    cmd->bindTexture(
      0, resources.getTexture(mip.textureName), linearClampSampler);
    cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);

    cmd->endRenderPass();

    device.submit(m_cmdBuffer);
  }

  // =========================================================================
  // Stage 4: Final combine pass
  // =========================================================================
  postProcessUBO.resolution.z = 1.0f; // exposure
  resources.flushPostProcessUBO();

  cmd->reset();

  gfx::RenderPassBeginInfo combinePassInfo{};
  combinePassInfo.framebuffer = bloomFinalFBO;
  combinePassInfo.clearColors[0] = glm::vec4(0.0f);
  combinePassInfo.clearDepth = 1.0f;
  combinePassInfo.clearStencil = 0;
  combinePassInfo.colorAttachmentCount = 1;
  combinePassInfo.clearColor = true;
  combinePassInfo.clearDepthStencil = true;
  cmd->beginRenderPass(combinePassInfo);

  cmd->bindPipeline(m_combinePipeline);

  cmd->setViewport(fullViewport);

  // Set sampler uniform bindings
  cmd->setUniform(m_combineSceneLoc, 0);
  cmd->setUniform(m_combineBloomBlurLoc, 1);

  cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);
  cmd->bindTexture(0, resources.getTexture("cubeFrame"), linearClampSampler);
  cmd->bindTexture(1,
                   resources.getTexture(m_mipChain.front().textureName),
                   linearClampSampler);
  cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);

  cmd->endRenderPass();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif

  device.submit(m_cmdBuffer);
}

void
BloomPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  auto& resources = gfx::RenderResources::getInstance();

  // Update mip chain sizes
  glm::vec2 currentMipSize(m_width, m_height);
  glm::ivec2 currentMipSizeInt(m_width, m_height);
  for (u32 i = 0; i < 5; i++) {
    currentMipSize *= 0.5f;
    currentMipSizeInt /= 2;
    m_mipChain[i].size = currentMipSize;
    m_mipChain[i].intSize = currentMipSizeInt;
  }

  // Recreate frameBright texture with new size
  if (m_useNewResources) {
    resources.recreateTexture2D("frameBright", m_width, m_height);
  }

  // Attach frameBright to brightFBO
  resources.setFramebufferAttachment("brightFBO", 0, "frameBright");
  std::array<u32, 1> drawBuffers = { 0 };
  resources.setDrawBuffers("brightFBO", drawBuffers);

  if (!resources.isFramebufferComplete("brightFBO")) {
    std::cout << "brightFBO not complete!\n";
  }

  // Recreate mip chain textures through RenderResources
  for (u32 i = 0; i < 5; i++) {
    mipLevel& mip = m_mipChain[i];
    if (m_useNewResources) {
      resources.recreateTexture2D(mip.textureName,
                                  static_cast<u32>(mip.size.x),
                                  static_cast<u32>(mip.size.y));
    }
  }

  // Attach first mip to bloomFBO
  resources.setFramebufferAttachment("bloomFBO", 0, m_mipChain[0].textureName);
  resources.setDrawBuffers("bloomFBO", drawBuffers);

  if (!resources.isFramebufferComplete("bloomFBO")) {
    std::cout << "bloomFBO not complete!\n";
    assert(false);
  }

  // Recreate frameBloomFinal texture with new size
  if (m_useNewResources) {
    resources.recreateTexture2D("frameBloomFinal", m_width, m_height);
  }

  // Attach frameBloomFinal to bloomFinalFBO
  resources.setFramebufferAttachment("bloomFinalFBO", 0, "frameBloomFinal");
  resources.setDrawBuffers("bloomFinalFBO", drawBuffers);

  if (!resources.isFramebufferComplete("bloomFinalFBO")) {
    std::cout << "bloomFinalFBO not complete!\n";
  }
}
