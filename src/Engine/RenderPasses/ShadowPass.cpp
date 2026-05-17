#include "ShadowPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "LightingUtil.hpp"

#include "ECS/ECSManager.hpp"
#include "ECS/Systems/CameraSystem.hpp"
#include "RenderPasses/RenderPass.hpp"
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <unordered_map>

namespace {

struct InstanceKey
{
  GraphicsObject* obj;
  u32 nodeIdx;
  u32 primIdx;

  bool operator==(const InstanceKey&) const = default;
};

struct InstanceKeyHash
{
  size_t operator()(const InstanceKey& key) const
  {
    auto h1 = std::hash<void*>{}(key.obj);
    auto h2 = std::hash<u32>{}(key.nodeIdx);
    auto h3 = std::hash<u32>{}(key.primIdx);
    return h1 ^ (h2 << 16) ^ (h3 << 32);
  }
};

struct DrawGroup
{
  InstanceKey key;
  u32 offset;
  u32 count;
};

// Shadow shader only uses locations 0, 4, 5 for mesh data, but the VBO is
// the same interleaved format as the geometry pass (72 bytes).
constexpr u32 kMeshVertexStride = 72;
constexpr u32 kInstanceStride = sizeof(glm::mat4); // 64 bytes

} // namespace

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
  m_isSkinnedLoc = device.getUniformLocation(getShaderId(), "is_skinned");

  // Set jointMats sampler uniform to texture unit 5 (for skinning)
  constexpr i32 kJointMatsUnit = 5;
  device.setUniformInt(device.getUniformLocation(getShaderId(), "jointMats"),
                       kJointMatsUnit);

  // Pipeline with full instanced vertex layout (same as GeometryPass).
  // Binding 0: mesh vertex data (per-vertex, stride 72)
  // Binding 1: instance model matrix (per-instance, stride 64)
  std::array<gfx::VertexBinding, 2> pipeBindings = {
    { { .binding = 0, .stride = kMeshVertexStride, .perInstance = false },
      { .binding = 1, .stride = kInstanceStride, .perInstance = true } }
  };
  // Shadow shader only reads POSITION(0), JOINTS_0(4), WEIGHTS_0(5) from mesh,
  // but we define all 6 mesh attributes to match the interleaved VBO layout.
  std::array<gfx::VertexAttribute, 7> pipeAttribs = { {
    { .location = 0,
      .binding = 0,
      .offset = 0,
      .format = gfx::PixelFormat::RGB32F }, // position
    { .location = 1,
      .binding = 0,
      .offset = 12,
      .format = gfx::PixelFormat::RGB32F }, // normal
    { .location = 2,
      .binding = 0,
      .offset = 24,
      .format = gfx::PixelFormat::RGBA32F }, // tangent
    { .location = 3,
      .binding = 0,
      .offset = 40,
      .format = gfx::PixelFormat::RG32F }, // texcoord
    { .location = 4,
      .binding = 0,
      .offset = 48,
      .format = gfx::PixelFormat::RGBA16 }, // joints
    { .location = 5,
      .binding = 0,
      .offset = 56,
      .format = gfx::PixelFormat::RGBA32F }, // weights
    { .location = 6,
      .binding = 1,
      .offset = 0,
      .format = gfx::PixelFormat::MAT4F }, // modelMatrix
  } };
  gfx::PipelineCreateInfo pipeInfo{};
  pipeInfo.shaderProgram = getShaderId();
  pipeInfo.vertexBindings = pipeBindings;
  pipeInfo.vertexAttributes = pipeAttribs;
  pipeInfo.topology = gfx::PrimitiveTopology::Triangles;
  pipeInfo.depthStencil.depthTestEnable = true;
  pipeInfo.depthStencil.depthWriteEnable = true;
  pipeInfo.depthStencil.depthCompareOp = gfx::CompareOp::Less;
  pipeInfo.blend.attachments[0].blendEnable = false;
  pipeInfo.rasterizer.cullMode = gfx::CullMode::Back;
  pipeInfo.debugName = "ShadowPassPipeline";
  m_pipeline = resources.createPipeline("ShadowPassPipeline", pipeInfo);

  // Create instance buffer
  gfx::BufferCreateInfo instanceBufInfo{};
  instanceBufInfo.size =
    static_cast<u64>(kInitialInstanceCapacity) * kInstanceStride;
  instanceBufInfo.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
  instanceBufInfo.debugName = "ShadowInstanceBuffer";
  m_instanceBuffer = device.createBuffer(instanceBufInfo);
  m_instanceBufferCapacity = kInitialInstanceCapacity;

  // Single-instance buffer for skinned entity draws (1 mat4)
  gfx::BufferCreateInfo singleBufInfo{};
  singleBufInfo.size = kInstanceStride;
  singleBufInfo.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
  singleBufInfo.debugName = "ShadowSingleInstanceBuffer";
  m_singleInstanceBuffer = device.createBuffer(singleBufInfo);
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
  auto& device = gfx::GraphicsDevice::getInstance();

  // Get camera for cascade calculation
  auto cam = CameraSystem::getInstance().getMainCameraComponent();

  // Get directional light
  glm::vec3 lightDirection(0.0f, -1.0f, 0.0f);
  std::vector<Entity> lightView = eManager.view<LightingComponent>();
  for (auto lightEntity : lightView) {
    auto* lightComp = eManager.getComponent<LightingComponent>(lightEntity);

    if (lightComp->type == LightingComponent::TYPE::DIRECTIONAL) {
      auto& light = static_cast<DirectionalLight&>(*lightComp->light);
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

  // Get entity list and sort into instanced vs skinned (once for all cascades)
  std::vector<Entity> entityView = eManager.view<GraphicsComponent>();

  std::unordered_map<InstanceKey, std::vector<glm::mat4>, InstanceKeyHash>
    instanceGroups;
  std::vector<Entity> skinnedEntities;

  for (auto entity : entityView) {
    auto* posComp = eManager.getComponent<PositionComponent>(entity);
    auto* gfxComp = eManager.getComponent<GraphicsComponent>(entity);
    auto* obj = gfxComp->m_grapObj.get();

    glm::mat4 entityModel =
      posComp ? glm::translate(glm::mat4(1.0f), posComp->position) *
                  glm::mat4_cast(posComp->rotation) *
                  glm::scale(glm::mat4(1.0f), posComp->scale)
              : glm::identity<glm::mat4>();

    bool hasSkin = false;
    for (u32 idx = 0; idx < obj->p_numNodes; idx++) {
      if (obj->p_nodes[idx].skin >= 0) {
        hasSkin = true;
        break;
      }
    }

    if (hasSkin) {
      skinnedEntities.push_back(entity);
    } else {
      for (u32 nodeIdx = 0; nodeIdx < obj->p_numNodes; nodeIdx++) {
        if (obj->p_nodes[nodeIdx].mesh < 0) {
          continue;
        }
        glm::mat4 nodeModel = entityModel * obj->getMatrix(nodeIdx);
        Mesh& mesh = obj->p_meshes[obj->p_nodes[nodeIdx].mesh];
        for (u32 primIdx = 0; primIdx < mesh.numPrims; primIdx++) {
          instanceGroups[{ obj, nodeIdx, primIdx }].push_back(nodeModel);
        }
      }
    }
  }

  // Build contiguous matrix buffer and draw groups
  static thread_local std::vector<glm::mat4> allMatrices;
  static thread_local std::vector<DrawGroup> drawGroups;
  allMatrices.clear();
  drawGroups.clear();

  for (auto& [key, matrices] : instanceGroups) {
    drawGroups.push_back({ key,
                           static_cast<u32>(allMatrices.size()),
                           static_cast<u32>(matrices.size()) });
    allMatrices.insert(allMatrices.end(), matrices.begin(), matrices.end());
  }

  // Upload all instance matrices (shared across all cascades)
  if (!allMatrices.empty()) {
    auto totalSize = static_cast<u32>(allMatrices.size()) * kInstanceStride;

    if (static_cast<u32>(allMatrices.size()) > m_instanceBufferCapacity) {
      device.destroyBuffer(m_instanceBuffer);
      m_instanceBufferCapacity = static_cast<u32>(allMatrices.size()) * 2;
      gfx::BufferCreateInfo info{};
      info.size = static_cast<u64>(m_instanceBufferCapacity) * kInstanceStride;
      info.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
      info.debugName = "ShadowInstanceBuffer";
      m_instanceBuffer = device.createBuffer(info);
    }

    device.updateBuffer(m_instanceBuffer, 0, allMatrices.data(), totalSize);
  }

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

    // Instanced draws (non-skinned)
    cmd->setUniform(m_isSkinnedLoc, 0);

    for (auto& group : drawGroups) {
      auto* obj = group.key.obj;
      Mesh& mesh = obj->p_meshes[obj->p_nodes[group.key.nodeIdx].mesh];
      Primitive& prim = mesh.m_primitives[group.key.primIdx];

      // Bind per-primitive VAO (handles binding 0 with correct stride/offsets).
      // Binding 1 (instance data) falls through to the pipeline's vertex
      // layout.
      cmd->bindVertexArray(prim.m_vaoId);
      cmd->bindVertexBuffer(0, prim.m_vboId);
      cmd->bindVertexBuffer(
        1, m_instanceBuffer, static_cast<u64>(group.offset) * kInstanceStride);

      if (device.isValid(prim.m_eboId)) {
        cmd->bindIndexBuffer(prim.m_eboId, 0, prim.m_indexType);
        cmd->drawIndexed(prim.m_count, group.count, prim.m_offset);
      } else {
        cmd->draw(prim.m_count, group.count);
      }
    }

    // Skinned draws (1-instance draws via instance buffer)
    for (auto entity : skinnedEntities) {
      auto* posComp = eManager.getComponent<PositionComponent>(entity);
      auto* gfxComp = eManager.getComponent<GraphicsComponent>(entity);

      glm::mat4 entityModel =
        posComp ? glm::translate(glm::mat4(1.0f), posComp->position) *
                    glm::mat4_cast(posComp->rotation) *
                    glm::scale(glm::mat4(1.0f), posComp->scale)
                : glm::identity<glm::mat4>();

      gfxComp->m_grapObj->recordDrawGeom(
        *cmd, entityModel, m_singleInstanceBuffer, m_isSkinnedLoc);
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
