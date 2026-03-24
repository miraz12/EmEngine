#include "GeometryPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/CameraSystem.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/UBOStructs.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <RenderPasses/RenderPass.hpp>

GeometryPass::GeometryPass()
  : RenderPass("GeometryPass",
               "resources/Shaders/mesh.vert",
               "resources/Shaders/pbrMesh.frag")
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  // Create FBO through new RenderResources system (bare FBO, attachments
  // managed in setViewport)
  resources.createBareFramebuffer("gBuffer");

  // Create renderbuffer for depth-stencil through new RenderResources system
  gfx::RenderbufferCreateInfo rboInfo{};
  rboInfo.width = m_width;
  rboInfo.height = m_height;
  rboInfo.format = gfx::PixelFormat::Depth24Stencil8;
  rboInfo.debugName = "gBufferDepth";
  resources.createRenderbuffer("gBufferDepth", rboInfo);

  // Create G-buffer textures through new RenderResources system
  // These are read by LightPass
  gfx::TextureCreateInfo gBufferInfo{};
  gBufferInfo.width = m_width;
  gBufferInfo.height = m_height;
  gBufferInfo.format = gfx::PixelFormat::RGBA16F;
  gBufferInfo.mipLevels = 1;

  gBufferInfo.debugName = "gPositionAo";
  resources.createTexture2D("gPositionAo", gBufferInfo);

  gBufferInfo.debugName = "gNormalMetal";
  resources.createTexture2D("gNormalMetal", gBufferInfo);

  gBufferInfo.debugName = "gAlbedoSpecRough";
  resources.createTexture2D("gAlbedoSpecRough", gBufferInfo);

  gBufferInfo.debugName = "gEmissive";
  resources.createTexture2D("gEmissive", gBufferInfo);

  // Create material sampler for texture binding
  gfx::SamplerCreateInfo samplerInfo{};
  samplerInfo.minFilter = gfx::FilterMode::LinearMipmapLinear;
  samplerInfo.magFilter = gfx::FilterMode::Linear;
  samplerInfo.wrapU = gfx::WrapMode::Repeat;
  samplerInfo.wrapV = gfx::WrapMode::Repeat;
  samplerInfo.debugName = "GeometryPass_MaterialSampler";
  m_sampler = device.createSampler(samplerInfo);

  m_useNewResources = true;

  setViewport(m_width, m_height);

  // Bind uniform blocks
  useShader();
  resources.bindShaderUniformBlock(
    getShaderId(), "CameraData", gfx::UBOBinding::Camera);
  resources.bindShaderUniformBlock(
    getShaderId(), "MaterialData", gfx::UBOBinding::Material);

  // Set texture sampler uniform once (always {0,1,2,3,4})
  constexpr i32 kNumMaterialTextures = 5;
  std::array<i32, kNumMaterialTextures> texUnits = { 0, 1, 2, 3, 4 };
  device.setUniformIntArray(
    device.getUniformLocation(getShaderId(), "textures"), texUnits);

  // Set jointMats sampler uniform to texture unit 5 (for skinning)
  constexpr i32 kJointMatsUnit = 5;
  i32 jointMatsLoc = device.getUniformLocation(getShaderId(), "jointMats");
  device.setUniformInt(jointMatsLoc, kJointMatsUnit);

  // Cache uniform locations for modelMatrix and is_skinned
  m_modelMatrixLoc = device.getUniformLocation(getShaderId(), "modelMatrix");
  m_isSkinnedLoc = device.getUniformLocation(getShaderId(), "is_skinned");

  // Create pipeline for CommandBuffer rendering.
  // Note: We use per-primitive VAOs (bindVertexArray), so the pipeline's vertex
  // layout is not used for actual drawing. We provide an empty layout here.
  gfx::PipelineCreateInfo pipeInfo{};
  pipeInfo.shaderProgram = getShaderId();
  // No vertex bindings/attributes - each primitive binds its own VAO
  pipeInfo.topology = gfx::PrimitiveTopology::Triangles;
  pipeInfo.depthStencil.depthTestEnable = true;
  pipeInfo.depthStencil.depthWriteEnable = true;
  pipeInfo.depthStencil.depthCompareOp = gfx::CompareOp::Less;
  pipeInfo.blend.attachments[0].blendEnable = false;
  pipeInfo.rasterizer.cullMode =
    gfx::CullMode::Back; // Default, may be overridden by material
  pipeInfo.debugName = "GeometryPassPipeline";
  m_pipeline = resources.createPipeline("GeometryPassPipeline", pipeInfo);

  resources.bindDefaultFramebuffer();
}

void
GeometryPass::Init(FrameGraph& fGraph)
{
  fGraph.getPass(PassId::kLight)->addTexture("gPositionAo");
  fGraph.getPass(PassId::kLight)->addTexture("gNormalMetal");
  fGraph.getPass(PassId::kLight)->addTexture("gAlbedoSpecRough");
  fGraph.getPass(PassId::kLight)->addTexture("gEmissive");
}

void
GeometryPass::Execute(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  // Update CameraUBO with current camera matrices (before CommandBuffer
  // recording)
  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());
  CameraSystem::updateCameraUBO(cam);

  // Ensure jointMats sampler uniform is set correctly each frame
  // (binding the shader and setting the uniform before recording)
  useShader();
  constexpr i32 kJointMatsUnit = 5;
  device.setUniformInt(device.getUniformLocation(getShaderId(), "jointMats"),
                       kJointMatsUnit);

  // Get command buffer for this pass
  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->pushDebugGroup("Geometry Pass");
#endif

  // Begin render pass with gBuffer framebuffer
  gfx::RenderPassBeginInfo passInfo{};
  passInfo.framebuffer = resources.getFramebuffer("gBuffer");
  passInfo.clearColors[0] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  passInfo.clearColors[1] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  passInfo.clearColors[2] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  passInfo.clearColors[3] = glm::vec4(0.0f, 0.0f, 0.0f, 0.0f);
  passInfo.clearDepth = 1.0f;
  passInfo.clearStencil = 0;
  passInfo.colorAttachmentCount = 4;
  passInfo.clearColor = true;
  passInfo.clearDepthStencil = true;
  cmd->beginRenderPass(passInfo);

  // Bind pipeline (includes shader, depth/stencil state, blend state)
  cmd->bindPipeline(m_pipeline);

  // Set viewport
  gfx::Viewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(m_width);
  viewport.height = static_cast<float>(m_height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  cmd->setViewport(viewport);

  // Iterate over entities with GraphicsComponent and record draw commands
  std::vector<Entity> view = eManager.view<GraphicsComponent>();
  for (auto entity : view) {
    std::shared_ptr<PositionComponent> posComp =
      eManager.getComponent<PositionComponent>(entity);
    std::shared_ptr<GraphicsComponent> gfxComp =
      eManager.getComponent<GraphicsComponent>(entity);

    // Set modelMatrix uniform
    if (posComp) {
      cmd->setUniform(m_modelMatrixLoc, posComp->model);
    } else {
      cmd->setUniform(m_modelMatrixLoc, glm::identity<glm::mat4>());
    }

    // Record draw commands (handles material binding + VAO + draw call +
    // skinning)
    gfxComp->m_grapObj->recordDraw(*cmd, m_sampler, m_isSkinnedLoc);
  }

  cmd->endRenderPass();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif

  // Submit the command buffer
  device.submit(m_cmdBuffer);
}

void
GeometryPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  auto& resources = gfx::RenderResources::getInstance();

  // Recreate G-buffer textures with new size
  if (m_useNewResources) {
    resources.recreateTexture2D("gPositionAo", m_width, m_height);
    resources.recreateTexture2D("gNormalMetal", m_width, m_height);
    resources.recreateTexture2D("gAlbedoSpecRough", m_width, m_height);
    resources.recreateTexture2D("gEmissive", m_width, m_height);
    resources.resizeRenderbuffer("gBufferDepth", m_width, m_height);
  }

  // Attach G-buffer textures to FBO
  resources.setFramebufferAttachment("gBuffer", 0, "gPositionAo");
  resources.setFramebufferAttachment("gBuffer", 1, "gNormalMetal");
  resources.setFramebufferAttachment("gBuffer", 2, "gAlbedoSpecRough");
  resources.setFramebufferAttachment("gBuffer", 3, "gEmissive");

  // Set draw buffers
  std::array<u32, 4> drawBuffers = { 0, 1, 2, 3 };
  resources.setDrawBuffers("gBuffer", drawBuffers);

  // Attach depth-stencil renderbuffer
  resources.setFramebufferRenderbuffer(
    "gBuffer", gfx::RenderbufferAttachment::DepthStencil, "gBufferDepth");

  // Check framebuffer completeness
  if (!resources.isFramebufferComplete("gBuffer")) {
    std::cout << "Framebuffer not complete!\n";
  }
}
