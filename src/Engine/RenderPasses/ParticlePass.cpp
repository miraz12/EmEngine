#include "ParticlePass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include <ECS/Components/ParticlesComponent.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/Resources/Pipeline.hpp>

ParticlePass::ParticlePass()
  : RenderPass("ParticlePass",
               "resources/Shaders/particle.vert",
               "resources/Shaders/particle.frag")
{
  auto& resources = gfx::RenderResources::getInstance();

  // Cache uniform locations for performance
  useShader();
  m_projMatrixLoc = getUniformLocation("projMatrix");
  m_viewMatrixLoc = getUniformLocation("viewMatrix");
  m_particlePosLoc = getUniformLocation("particlePos");
  m_colorLoc = getUniformLocation("color");

  // Create pipeline for particle rendering
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
  pipeInfo.depthStencil.depthTestEnable = true;
  pipeInfo.depthStencil.depthWriteEnable =
    false; // Particles don't write to depth
  pipeInfo.blend.attachments[0].blendEnable = true;
  pipeInfo.blend.attachments[0].srcColorBlendFactor =
    gfx::BlendFactor::SrcAlpha;
  pipeInfo.blend.attachments[0].dstColorBlendFactor =
    gfx::BlendFactor::OneMinusSrcAlpha;
  pipeInfo.rasterizer.cullMode = gfx::CullMode::None;
  pipeInfo.debugName = "ParticlePassPipeline";
  m_pipeline = resources.createPipeline("ParticlePassPipeline", pipeInfo);
}

void
ParticlePass::Record(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();

  // Get command buffer for this pass
  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());

  // Begin render pass - particles render to cubeFBO (same as CubeMapPass
  // output)
  gfx::RenderPassBeginInfo passInfo{};
  passInfo.framebuffer = resources.getFramebuffer("cubeFBO");
  passInfo.colorAttachmentCount = 1;
  passInfo.clearColor = false; // Don't clear, blend on top of existing content
  passInfo.clearDepthStencil = false;

  cmd->pushDebugGroup("Particle Pass");
  cmd->beginRenderPass(passInfo);

  // Bind pipeline (includes shader, blend enabled, depth test but no depth
  // write)
  cmd->bindPipeline(m_pipeline);

  // Set viewport
  gfx::Viewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(m_width);
  viewport.height = static_cast<float>(m_height);
  cmd->setViewport(viewport);

  // Set projection and view matrices
  cmd->setUniform(m_projMatrixLoc, cam->m_ProjectionMatrix);
  cmd->setUniform(m_viewMatrixLoc, cam->m_viewMatrix);

  // Bind quad vertex buffer once
  cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);

  // Record draw commands for all alive particles
  std::vector<Entity> view = eManager.view<ParticlesComponent>();
  for (auto& entity : view) {
    std::shared_ptr<ParticlesComponent> pComp =
      eManager.getComponent<ParticlesComponent>(entity);
    for (auto& particle : pComp->getAliveParticles()) {
      if (particle->life > 0.0f) {
        cmd->setUniform(m_particlePosLoc, particle->position);
        cmd->setUniform(m_colorLoc, particle->color);
        cmd->draw(gfx::RenderResources::kQuadVertexCount, 1, 0, 0);
      }
    }
  }

  cmd->endRenderPass();
  cmd->popDebugGroup();
}

void
ParticlePass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;
}
