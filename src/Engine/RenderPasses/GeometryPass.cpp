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
#include <unordered_map>

namespace {

// Instance key for grouping entities that share the same primitive
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
  u32 offset; // index into allMatrices
  u32 count;  // number of instances
};

// Mesh vertex stride (must match InterleavedVertex in GltfObject.cpp)
// vec3 pos + vec3 norm + vec4 tan + vec2 uv + u16vec4 joints + vec4 weights
constexpr u32 kMeshVertexStride = 72;
constexpr u32 kInstanceStride = sizeof(glm::mat4); // 64 bytes

} // namespace

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

  // Cache uniform locations
  m_isSkinnedLoc = device.getUniformLocation(getShaderId(), "is_skinned");

  // Pipeline with full instanced vertex layout.
  // Binding 0: mesh vertex data (per-vertex, locations 0-5)
  // Binding 1: instance model matrix (per-instance, locations 6-9)
  // For non-instanced (skinned) draws, per-primitive VAOs override binding 0.
  // For instanced draws, the pipeline layout is used for both bindings.
  std::array<gfx::VertexBinding, 2> pipeBindings = {
    { { .binding = 0, .stride = kMeshVertexStride, .perInstance = false },
      { .binding = 1, .stride = kInstanceStride, .perInstance = true } }
  };
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
  pipeInfo.debugName = "GeometryPassPipeline";
  m_pipeline = resources.createPipeline("GeometryPassPipeline", pipeInfo);

  // Create instance buffer for batched model matrices
  gfx::BufferCreateInfo instanceBufInfo{};
  instanceBufInfo.size =
    static_cast<u64>(kInitialInstanceCapacity) * kInstanceStride;
  instanceBufInfo.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
  instanceBufInfo.debugName = "GeometryInstanceBuffer";
  m_instanceBuffer = device.createBuffer(instanceBufInfo);
  m_instanceBufferCapacity = kInitialInstanceCapacity;

  // Single-instance buffer for skinned entity draws (1 mat4)
  gfx::BufferCreateInfo singleBufInfo{};
  singleBufInfo.size = kInstanceStride;
  singleBufInfo.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
  singleBufInfo.debugName = "GeometrySingleInstanceBuffer";
  m_singleInstanceBuffer = device.createBuffer(singleBufInfo);

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
GeometryPass::Record(ECSManager& eManager)
{
  auto& resources = gfx::RenderResources::getInstance();
  auto& device = gfx::GraphicsDevice::getInstance();

  // Update CameraUBO with current camera matrices (before CommandBuffer
  // recording)
  auto cam = CameraSystem::getInstance().getMainCameraComponent();
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

  // Phase 1: Sort entities into instance groups vs non-instanced (skinned)
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

    // Check if any node has skinning
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
      // Group by (obj, node, primitive) for instancing
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

  // Phase 2: Build contiguous matrix buffer and draw groups
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

  // Upload all instance matrices
  if (!allMatrices.empty()) {
    auto totalSize = static_cast<u32>(allMatrices.size()) * kInstanceStride;

    // Resize buffer if needed
    if (static_cast<u32>(allMatrices.size()) > m_instanceBufferCapacity) {
      device.destroyBuffer(m_instanceBuffer);
      m_instanceBufferCapacity = static_cast<u32>(allMatrices.size()) * 2;
      gfx::BufferCreateInfo info{};
      info.size = static_cast<u64>(m_instanceBufferCapacity) * kInstanceStride;
      info.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
      info.debugName = "GeometryInstanceBuffer";
      m_instanceBuffer = device.createBuffer(info);
    }

    device.updateBuffer(m_instanceBuffer, 0, allMatrices.data(), totalSize);
  }

  // Phase 3: Issue instanced draw calls
  cmd->setUniform(m_isSkinnedLoc, 0);

  for (auto& group : drawGroups) {
    auto* obj = group.key.obj;
    Mesh& mesh = obj->p_meshes[obj->p_nodes[group.key.nodeIdx].mesh];
    Primitive& prim = mesh.m_primitives[group.key.primIdx];

    // Bind material
    Material* mat = prim.m_material > -1 ? &obj->p_materials[prim.m_material]
                                         : &obj->defaultMat;
    mat->recordBind(*cmd, m_sampler);

    // Bind per-primitive VAO (handles binding 0 with correct stride/offsets).
    // Binding 1 (instance data) falls through to the pipeline's vertex layout.
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

  // Phase 4: Draw skinned entities (1-instance draws via instance buffer)
  for (auto entity : skinnedEntities) {
    auto* posComp = eManager.getComponent<PositionComponent>(entity);
    auto* gfxComp = eManager.getComponent<GraphicsComponent>(entity);

    glm::mat4 entityModel =
      posComp ? glm::translate(glm::mat4(1.0f), posComp->position) *
                  glm::mat4_cast(posComp->rotation) *
                  glm::scale(glm::mat4(1.0f), posComp->scale)
              : glm::identity<glm::mat4>();

    gfxComp->m_grapObj->recordDraw(
      *cmd, m_sampler, entityModel, m_singleInstanceBuffer, m_isSkinnedLoc);
  }

  cmd->endRenderPass();

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  cmd->popDebugGroup();
#endif
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
