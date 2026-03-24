#include "FxaaPass.hpp"
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/UBOStructs.hpp>
#include <array>

FxaaPass::FxaaPass()
  : RenderPass("FxaaPass",
               "resources/Shaders/quad.vert",
               "resources/Shaders/Fxaa.frag")
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  useShader();
  gfx::ShaderId shader = getShaderId();

  // Bind PostProcessData UBO block
  resources.bindShaderUniformBlock(
    shader, "PostProcessData", gfx::UBOBinding::PostProcess);

  // Set sampler uniform (texture unit 0)
  device.setUniformInt(device.getUniformLocation(shader, "scene"), 0);

  // Create pipeline for fullscreen quad rendering (for future use)
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

  gfx::PipelineCreateInfo pipeInfo{};
  pipeInfo.shaderProgram = shader;
  pipeInfo.vertexBindings = bindings;
  pipeInfo.vertexAttributes = attributes;
  pipeInfo.topology = gfx::PrimitiveTopology::TriangleStrip;
  pipeInfo.depthStencil.depthTestEnable = false;
  pipeInfo.depthStencil.depthWriteEnable = false;
  pipeInfo.blend.attachments[0].blendEnable = false;
  pipeInfo.rasterizer.cullMode = gfx::CullMode::None;
  pipeInfo.debugName = "FxaaPassPipeline";
  m_pipeline = resources.createPipeline("FxaaPassPipeline", pipeInfo);
}

void
FxaaPass::Execute(ECSManager& /* eManager */)
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  // Set resolution in PostProcessUBO and flush before CommandBuffer
  gfx::PostProcessUBO& postProcessUBO = resources.getPostProcessUBO();
  postProcessUBO.resolution = glm::vec4(
    static_cast<float>(m_width), static_cast<float>(m_height), 1.0f, 0.0f);
  resources.flushPostProcessUBO();

  // Get command buffer for this pass
  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->pushDebugGroup("FXAA Pass");
#endif

  // Begin render pass (default framebuffer with clear)
  gfx::RenderPassBeginInfo passInfo{};
  passInfo.framebuffer = device.getDefaultFramebuffer();
  passInfo.clearColors[0] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  passInfo.clearDepth = 1.0f;
  passInfo.clearStencil = 0;
  passInfo.colorAttachmentCount = 1;
  passInfo.clearColor = true;
  passInfo.clearDepthStencil = true;
  cmd->beginRenderPass(passInfo);

  // Bind pipeline
  cmd->bindPipeline(m_pipeline);

  // Set sampler uniform (must be done after pipeline bind since it uses the
  // shader)
  cmd->setUniform(getUniformLocation("scene"), 0);

  // Set viewport
  gfx::Viewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(m_width);
  viewport.height = static_cast<float>(m_height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  cmd->setViewport(viewport);

  // Bind vertex buffer
  cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);

  // Bind input texture
  gfx::TextureId tex = resources.getTexture("frameBloomFinal");
  gfx::SamplerId sampler = resources.getLinearClampSampler();

  cmd->bindTexture(0, tex, sampler);

  // Draw fullscreen quad
  cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);

  cmd->endRenderPass();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif

  // Submit the command buffer
  device.submit(m_cmdBuffer);
}

void
FxaaPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;
}
