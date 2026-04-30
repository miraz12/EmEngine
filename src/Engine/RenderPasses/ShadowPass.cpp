#include "ShadowPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "LightingUtil.hpp"

#include "ECS/ECSManager.hpp"
#include "RenderPasses/RenderPass.hpp"
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <RenderPasses/FrameGraph.hpp>

ShadowPass::ShadowPass()
  : RenderPass("ShadowPass",
               "resources/Shaders/shadow.vert",
               "resources/Shaders/shadow.frag")
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  // Create UBO for cascade data through RenderResources
  // Size: 4 mat4 + 1 vec4 + 1 ivec4 = 288 bytes, binding point 0
  constexpr u32 kCascadeUBOSize = 288;
  resources.createUniformBuffer("cascadeUBO", kCascadeUBOSize, 0);

  // Create FBO through new RenderResources system (bare FBO, attachments
  // managed separately)
  resources.createBareFramebuffer("depthMapFbo");

  // Create cascaded shadow map array through new RenderResources system
  // This is read by LightPass
  gfx::TextureCreateInfo shadowInfo{};
  shadowInfo.width = SHADOW_MAP_SIZE;
  shadowInfo.height = SHADOW_MAP_SIZE;
  shadowInfo.depthOrLayers = NUM_CASCADES;
  shadowInfo.format = gfx::PixelFormat::Depth24;
  shadowInfo.mipLevels = 1;
  shadowInfo.debugName = "depthMapArray";
  resources.createTexture2DArray("depthMapArray", shadowInfo);
  m_useNewResources = true;

  // Shadow comparison mode is handled by RenderResources::getShadowSampler()
  // which is bound in LightPass when sampling the shadow map

  // Attach first cascade layer for initial setup (will be rebound per cascade)
  resources.setFramebufferDepthAttachment("depthMapFbo", "depthMapArray", 0, 0);

  // No color attachments for depth-only FBO
  std::array<u32, 0> noColorAttachments = {};
  resources.setDrawBuffers("depthMapFbo", noColorAttachments);

  // Set read buffer to none for depth-only FBO
  resources.bindFramebuffer("depthMapFbo");
  resources.setReadBuffer(std::nullopt); // GL_NONE

  if (!resources.isFramebufferComplete("depthMapFbo")) {
    std::cout << "FB error, status: depthMapFbo incomplete\n";
  }
  resources.bindDefaultFramebuffer();

  // Cache uniform locations for CommandBuffer use
  useShader();
  m_lightSpaceMatrixLoc =
    device.getUniformLocation(getShaderId(), "lightSpaceMatrix");
  m_modelMatrixLoc = device.getUniformLocation(getShaderId(), "modelMatrix");
  m_isSkinnedLoc = device.getUniformLocation(getShaderId(), "is_skinned");

  // Set jointMats sampler uniform to texture unit 5 (for skinning)
  constexpr i32 kJointMatsUnit = 5;
  device.setUniformInt(device.getUniformLocation(getShaderId(), "jointMats"),
                       kJointMatsUnit);

  // Create pipeline for CommandBuffer rendering
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
  pipeInfo.rasterizer.cullMode = gfx::CullMode::Back;
  pipeInfo.debugName = "ShadowPassPipeline";
  m_pipeline = resources.createPipeline("ShadowPassPipeline", pipeInfo);
}

void
ShadowPass::Init(FrameGraph& fGraph)
{
  fGraph.getPass(PassId::kLight)->addTexture("depthMapArray");
}

void
ShadowPass::Record(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();

  // Get camera for cascade calculation
  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  // Get directional light
  glm::vec3 lightDirection(0.0f, -1.0f, 0.0f);
  std::vector<Entity> lightView = eManager.view<LightingComponent>();
  for (auto lightEntity : lightView) {
    std::shared_ptr<LightingComponent> lightComp =
      eManager.getComponent<LightingComponent>(lightEntity);

    if (lightComp->getType() == LightingComponent::TYPE::DIRECTIONAL) {
      auto& light = static_cast<DirectionalLight&>(lightComp->getBaseLight());
      lightDirection = light.direction;
      break;
    }
  }

  // Calculate cascade splits and matrices
  constexpr float kLambdaSplit = 0.5f;
  m_cascadeConfig.calculateSplitDistances(
    cam->m_near, cam->m_far, kLambdaSplit);
  m_cascadeConfig.calculateCascadeMatrices(lightDirection,
                                           cam->m_position,
                                           cam->m_viewMatrix,
                                           cam->m_ProjectionMatrix);

  // Update UBO with cascade data
  struct CascadeUBO
  {
    std::array<glm::mat4, NUM_CASCADES> lightSpaceMatrices;
    glm::vec4 cascadeSplits;
    glm::ivec4 config;
  } uboData;

  std::copy(std::begin(m_cascadeConfig.lightSpaceMatrices),
            std::end(m_cascadeConfig.lightSpaceMatrices),
            std::begin(uboData.lightSpaceMatrices));

  uboData.cascadeSplits = glm::vec4(m_cascadeConfig.cascadeSplits[0],
                                    m_cascadeConfig.cascadeSplits[1],
                                    m_cascadeConfig.cascadeSplits[2],
                                    m_cascadeConfig.cascadeSplits[3]);
  uboData.config = glm::ivec4(NUM_CASCADES, SHADOW_MAP_SIZE, 0, 0);

  // Update cascade UBO through RenderResources
  resources.updateUniformBuffer("cascadeUBO", &uboData, sizeof(CascadeUBO));

  // Get command buffer for this pass
  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->pushDebugGroup("Shadow Pass CSM");
#endif

  // Get entity list once for all cascades
  std::vector<Entity> entityView = eManager.view<GraphicsComponent>();

  // Get FBO and texture handles for CommandBuffer commands
  gfx::FramebufferId depthMapFbo = resources.getFramebuffer("depthMapFbo");
  gfx::TextureId depthMapArray = resources.getTexture("depthMapArray");

  // Set viewport for all cascades
  gfx::Viewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(SHADOW_MAP_SIZE);
  viewport.height = static_cast<float>(SHADOW_MAP_SIZE);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;

  // Render each cascade
  for (u32 cascade = 0; cascade < NUM_CASCADES; ++cascade) {
    // Attach this cascade's layer to the framebuffer
    cmd->setFramebufferDepthAttachment(depthMapFbo, depthMapArray, 0, cascade);

    // Begin render pass (clears depth)
    gfx::RenderPassBeginInfo passInfo{};
    passInfo.framebuffer = depthMapFbo;
    passInfo.clearDepth = 1.0f;
    passInfo.clearStencil = 0;
    passInfo.colorAttachmentCount = 0;
    passInfo.clearColor = false;
    passInfo.clearDepthStencil = true;
    cmd->beginRenderPass(passInfo);

    // Bind pipeline
    cmd->bindPipeline(m_pipeline);

    // Set viewport
    cmd->setViewport(viewport);

    // Set light space matrix for this cascade
    cmd->setUniform(m_lightSpaceMatrixLoc,
                    m_cascadeConfig.lightSpaceMatrices[cascade]);

    // Draw all shadow-casting geometry
    for (auto& entity : entityView) {
      std::shared_ptr<PositionComponent> posComp =
        eManager.getComponent<PositionComponent>(entity);

      // Set modelMatrix uniform
      if (posComp) {
        cmd->setUniform(m_modelMatrixLoc, posComp->model);
      } else {
        cmd->setUniform(m_modelMatrixLoc, glm::identity<glm::mat4>());
      }

      std::shared_ptr<GraphicsComponent> grapComp =
        eManager.getComponent<GraphicsComponent>(entity);

      // Record geometry-only draw commands (no material bindings, with skinning
      // support)
      grapComp->m_grapObj->recordDrawGeom(*cmd, m_isSkinnedLoc);
    }

    cmd->endRenderPass();
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif
}

void
ShadowPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  // Note: Shadow map resolution is fixed at SHADOW_MAP_SIZE, not viewport size
  // No need to recreate the texture - it's allocated with immutable storage
}
