#include "ParticlePass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include <ECS/Components/ParticlesComponent.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/Resources/Pipeline.hpp>

namespace {
struct ParticleInstanceData
{
  glm::vec3 position;
  glm::vec4 color;
};
static constexpr u32 kInstanceStride = sizeof(ParticleInstanceData); // 28 bytes
} // namespace

ParticlePass::ParticlePass()
  : RenderPass("ParticlePass",
               "resources/Shaders/particle.vert",
               "resources/Shaders/particle.frag")
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  // Cache uniform locations for performance
  useShader();
  m_projMatrixLoc = getUniformLocation("projMatrix");
  m_viewMatrixLoc = getUniformLocation("viewMatrix");

  // Create pipeline for instanced particle rendering
  // Binding 0: Quad VBO (per-vertex) — vec3 position, vec2 texcoord
  // Binding 1: Instance VBO (per-instance) — vec3 particlePos, vec4 color
  static constexpr u32 kQuadStride = 5 * sizeof(float);

  std::array<gfx::VertexBinding, 2> bindings = {
    { { .binding = 0, .stride = kQuadStride, .perInstance = false },
      { .binding = 1, .stride = kInstanceStride, .perInstance = true } }
  };
  std::array<gfx::VertexAttribute, 4> attributes = { {
    { .location = 0,
      .binding = 0,
      .offset = 0,
      .format = gfx::PixelFormat::RGB32F },
    { .location = 1,
      .binding = 0,
      .offset = 3 * sizeof(float),
      .format = gfx::PixelFormat::RG32F },
    { .location = 2,
      .binding = 1,
      .offset = 0,
      .format = gfx::PixelFormat::RGB32F },
    { .location = 3,
      .binding = 1,
      .offset = offsetof(ParticleInstanceData, color),
      .format = gfx::PixelFormat::RGBA32F },
  } };

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

  // Create initial instance buffer
  gfx::BufferCreateInfo instanceBufInfo{};
  instanceBufInfo.size = kInitialInstanceCapacity * kInstanceStride;
  instanceBufInfo.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
  instanceBufInfo.debugName = "ParticleInstanceBuffer";
  m_instanceBuffer = device.createBuffer(instanceBufInfo);
  m_instanceBufferCapacity = kInitialInstanceCapacity;
}

void
ParticlePass::Record(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  gfx::CommandBuffer* cmd = getCommandBuffer();
  if (cmd == nullptr) {
    return;
  }

  auto cam = CameraSystem::getInstance().getMainCameraComponent();

  // Phase 1: Gather all alive particles into contiguous instance data
  static thread_local std::vector<ParticleInstanceData> instanceData;
  instanceData.clear();

  std::vector<Entity> view = eManager.view<ParticlesComponent>();
  for (auto& entity : view) {
    auto* pComp = eManager.getComponent<ParticlesComponent>(entity);
    for (auto& particle : pComp->aliveParticles) {
      if (particle->life > 0.0f) {
        instanceData.push_back({ particle->position, particle->color });
      }
    }
  }

  auto particleCount = static_cast<u32>(instanceData.size());
  if (particleCount == 0) {
    return;
  }

  // Resize instance buffer if needed (2x amortization)
  if (particleCount > m_instanceBufferCapacity) {
    device.destroyBuffer(m_instanceBuffer);
    m_instanceBufferCapacity = particleCount * 2;
    gfx::BufferCreateInfo info{};
    info.size = m_instanceBufferCapacity * kInstanceStride;
    info.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
    info.debugName = "ParticleInstanceBuffer";
    m_instanceBuffer = device.createBuffer(info);
  }

  // Upload instance data
  device.updateBuffer(
    m_instanceBuffer, 0, instanceData.data(), particleCount * kInstanceStride);

  // Phase 2: Record a single instanced draw call
  cmd->pushDebugGroup("Particle Pass");

  gfx::RenderPassBeginInfo passInfo{};
  passInfo.framebuffer = resources.getFramebuffer("cubeFBO");
  passInfo.colorAttachmentCount = 1;
  passInfo.clearColor = false;
  passInfo.clearDepthStencil = false;
  cmd->beginRenderPass(passInfo);

  cmd->bindPipeline(m_pipeline);

  gfx::Viewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width = static_cast<float>(m_width);
  viewport.height = static_cast<float>(m_height);
  cmd->setViewport(viewport);

  cmd->setUniform(m_projMatrixLoc, cam->m_ProjectionMatrix);
  cmd->setUniform(m_viewMatrixLoc, cam->m_viewMatrix);

  // Bind quad VBO (binding 0) and instance VBO (binding 1)
  cmd->bindVertexBuffer(0, resources.getQuadVertexBuffer(), 0);
  cmd->bindVertexBuffer(1, m_instanceBuffer, 0);

  // One instanced draw for ALL particles
  cmd->draw(gfx::RenderResources::kQuadVertexCount, particleCount, 0, 0);

  cmd->endRenderPass();
  cmd->popDebugGroup();
}

void
ParticlePass::setViewport(u32 w, u32 h)
{
  m_width = w;
  m_height = h;
}
