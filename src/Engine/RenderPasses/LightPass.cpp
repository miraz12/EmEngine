#include "LightPass.hpp"
#include <ECS/Components/LightingComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/Resources/Pipeline.hpp>
#include <Graphics/UBOStructs.hpp>
#include <array>

LightPass::LightPass()
  : RenderPass("LightPass",
               "resources/Shaders/light.vert",
               "resources/Shaders/pbrLight.frag")
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  useShader();
  gfx::ShaderId shader = getShaderId();

  // Bind UBO blocks
  resources.bindShaderUniformBlock(
    shader, "CascadeData", gfx::UBOBinding::CascadeData);
  resources.bindShaderUniformBlock(
    shader, "CameraData", gfx::UBOBinding::Camera);
  resources.bindShaderUniformBlock(
    shader, "LightingData", gfx::UBOBinding::Lighting);

  // Set sampler uniform locations to match texture binding order.
  // Textures are populated by addTexture() calls from other passes' Init()
  // methods: CubeMapPass::Init: irradianceMap(0), prefilterMap(1), brdfLUT(2)
  // ShadowPass::Init: depthMapArray(3)
  // GeometryPass::Init: gPositionAo(4), gNormalMetal(5), gAlbedoSpecRough(6),
  // gEmissive(7)
  //
  // Each sampler uniform must be set to the texture unit it will be bound to.
  device.setUniformInt(device.getUniformLocation(shader, "irradianceMap"), 0);
  device.setUniformInt(device.getUniformLocation(shader, "prefilterMap"), 1);
  device.setUniformInt(device.getUniformLocation(shader, "brdfLUT"), 2);
  device.setUniformInt(device.getUniformLocation(shader, "depthMapArray"), 3);
  device.setUniformInt(device.getUniformLocation(shader, "gPositionAo"), 4);
  device.setUniformInt(device.getUniformLocation(shader, "gNormalMetal"), 5);
  device.setUniformInt(device.getUniformLocation(shader, "gAlbedoSpecRough"),
                       6);
  device.setUniformInt(device.getUniformLocation(shader, "gEmissive"), 7);

  // Create FBO through new RenderResources system (bare FBO, attachments
  // managed in setViewport)
  resources.createBareFramebuffer("lightFBO");

  // Create renderbuffer for depth through new RenderResources system
  gfx::RenderbufferCreateInfo rboInfo{};
  rboInfo.width = m_width;
  rboInfo.height = m_height;
  rboInfo.format = gfx::PixelFormat::Depth24Stencil8;
  rboInfo.debugName = "lightFBODepth";
  resources.createRenderbuffer("lightFBODepth", rboInfo);

  // Create lightFrame texture through new RenderResources system
  // This is read by CubeMapPass
  gfx::TextureCreateInfo lightFrameInfo{};
  lightFrameInfo.width = m_width;
  lightFrameInfo.height = m_height;
  lightFrameInfo.format = gfx::PixelFormat::RGBA16F;
  lightFrameInfo.mipLevels = 1;
  lightFrameInfo.debugName = "lightFrame";
  resources.createTexture2D("lightFrame", lightFrameInfo);

  m_useNewResources = true;

  // Create pipeline (for future CommandBuffer use)
  // Quad layout: vec3 position (loc 0), vec2 texcoord (loc 1)
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
  pipeInfo.shaderProgram = getShaderId();
  pipeInfo.vertexBindings = bindings;
  pipeInfo.vertexAttributes = attributes;
  pipeInfo.topology = gfx::PrimitiveTopology::TriangleStrip;
  pipeInfo.depthStencil.depthTestEnable = false;
  pipeInfo.depthStencil.depthWriteEnable = false;
  pipeInfo.blend.attachments[0].blendEnable = false;
  pipeInfo.rasterizer.cullMode = gfx::CullMode::None;
  pipeInfo.debugName = "LightPassPipeline";
  m_pipeline = resources.createPipeline("LightPassPipeline", pipeInfo);

  setViewport(m_width, m_height);
}

void
LightPass::Record(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();

  // Populate LightingUBO from ECS light components
  gfx::LightingUBO& lightingUBO = resources.getLightingUBO();

  std::vector<Entity> view = eManager.view<LightingComponent>();
  i32 numPLights = 0;
  for (auto e : view) {
    std::shared_ptr<LightingComponent> g =
      eManager.getComponent<LightingComponent>(e);

    switch (g->getType()) {
      case LightingComponent::TYPE::DIRECTIONAL: {
        auto& light = static_cast<DirectionalLight&>(g->getBaseLight());

        lightingUBO.dirLightDirection = glm::vec4(light.direction, 0.0f);
        lightingUBO.dirLightColor = glm::vec4(light.color, light.intensity);

        break;
      }
      case LightingComponent::TYPE::POINT: {
        if (numPLights >= static_cast<i32>(MAX_POINT_LIGHTS)) {
          break;
        }

        PointLight& light = static_cast<PointLight&>(g->getBaseLight());

        // Calculate radius for culling
        const float constant = 1.0f;
        float maxBrightness =
          std::fmaxf(std::fmaxf(light.color.r, light.color.g), light.color.b);
        float radius =
          (-light.linear +
           std::sqrt(light.linear * light.linear -
                     4 * light.quadratic *
                       (constant - (256.0f / 5.0f) * maxBrightness))) /
          (2.0f * light.quadratic);

        // Populate UBO point light data
        lightingUBO.pointLights[numPLights].positionRadius =
          glm::vec4(light.position, radius);
        lightingUBO.pointLights[numPLights].colorIntensity =
          glm::vec4(light.color, 0.0f);
        lightingUBO.pointLights[numPLights].attenuation =
          glm::vec4(light.constant, light.linear, light.quadratic, 0.0f);

        numPLights++;
        break;
      }
      default:
        break;
    }
  }

  // Set light config (numPointLights, debugView)
  lightingUBO.lightConfig =
    glm::ivec4(numPLights, eManager.getDebugView(), 0, 0);

  // Upload UBO to GPU before CommandBuffer recording
  resources.flushLightingUBO();

  // Get command buffer for this pass
  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->pushDebugGroup("Light Pass");
#endif

  // Begin render pass with lightFBO
  gfx::RenderPassBeginInfo passInfo{};
  passInfo.framebuffer = resources.getFramebuffer("lightFBO");
  passInfo.clearColors[0] = glm::vec4(0.2f, 0.3f, 0.3f, 1.0f);
  passInfo.clearDepth = 1.0f;
  passInfo.clearStencil = 0;
  passInfo.colorAttachmentCount = 1;
  passInfo.clearColor = true;
  passInfo.clearDepthStencil = false;
  cmd->beginRenderPass(passInfo);

  // Bind pipeline (includes shader, depth test disabled, no blending)
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

  // Bind vertex buffer for fullscreen quad
  cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);

  // Bind all input textures with appropriate samplers
  gfx::SamplerId linearClampSampler = resources.getLinearClampSampler();
  gfx::SamplerId linearMipmapClampSampler =
    resources.getLinearMipmapClampSampler();
  gfx::SamplerId shadowSampler = resources.getShadowSampler();

  for (size_t idx = 0; idx < m_textures.size(); idx++) {
    gfx::TextureId tex = resources.getTexture(m_textures[idx]);
    // Select appropriate sampler:
    // - Shadow sampler for depthMapArray (has comparison mode enabled)
    // - Mipmap sampler for IBL cubemaps that use textureLod() in shader
    // - Linear clamp for everything else
    gfx::SamplerId sampler = linearClampSampler;
    if (m_textures[idx] == "depthMapArray") {
      sampler = shadowSampler;
    } else if (m_textures[idx] == "irradianceMap" ||
               m_textures[idx] == "prefilterMap") {
      sampler = linearMipmapClampSampler;
    }
    cmd->bindTexture(static_cast<u32>(idx), tex, sampler);
  }

  // Draw fullscreen quad
  cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);

  cmd->endRenderPass();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif
}

void
LightPass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;

  auto& resources = gfx::RenderResources::getInstance();

  // Recreate lightFrame texture and renderbuffer with new size
  if (m_useNewResources) {
    resources.recreateTexture2D("lightFrame", m_width, m_height);
    resources.resizeRenderbuffer("lightFBODepth", m_width, m_height);
  }

  // Attach lightFrame texture to FBO
  resources.setFramebufferAttachment("lightFBO", 0, "lightFrame");

  // Set draw buffers
  std::array<u32, 1> drawBuffers = { 0 };
  resources.setDrawBuffers("lightFBO", drawBuffers);

  // Attach depth renderbuffer
  resources.setFramebufferRenderbuffer(
    "lightFBO", gfx::RenderbufferAttachment::Depth, "lightFBODepth");

  // Check framebuffer completeness
  if (!resources.isFramebufferComplete("lightFBO")) {
    std::cout << "Framebuffer not complete!\n";
  }
}
