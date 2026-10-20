#include "GeometryPass.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/GraphicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <ECS/Systems/CameraSystem.hpp>
#include <Graphics/CommandBuffer.hpp>
#include <Graphics/GraphicsDevice.hpp>
#include <Graphics/RenderResources.hpp>
#include <Graphics/UBOStructs.hpp>
#include <RenderPasses/FrameGraph.hpp>
#include <RenderPasses/RenderPass.hpp>
#include <RenderPasses/SkinningGather.hpp>
#include <Rendering/SkeletonPose.hpp>
#include <unordered_map>

namespace {

// Instance key for grouping entities that share the same primitive
struct InstanceKey
{
  GraphicsObject* obj;
  u32 nodeIdx;
  u32 primIdx;
  bool skinned;

  bool operator==(const InstanceKey&) const = default;
};

struct InstanceKeyHash
{
  size_t operator()(const InstanceKey& key) const
  {
    auto h1 = std::hash<void*>{}(key.obj);
    auto h2 = std::hash<u32>{}(key.nodeIdx);
    auto h3 = std::hash<u32>{}(key.primIdx);
    auto h4 = std::hash<bool>{}(key.skinned);
    return h1 ^ (h2 << 16) ^ (h3 << 32) ^ (h4 << 48);
  }
};

struct DrawGroup
{
  InstanceKey key;
  u32 offset; // index into allInstances
  u32 count;  // number of instances
};

// Mesh vertex stride (must match InterleavedVertex in GltfObject.cpp)
// vec3 pos + vec3 norm + vec4 tan + vec2 uv + u16vec4 joints + vec4 weights
constexpr u32 kMeshVertexStride = 72;
constexpr u32 kInstanceStride = sizeof(InstanceData);

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
  std::array<gfx::VertexAttribute, 8> pipeAttribs = { {
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
    { .location = 10,
      .binding = 1,
      .offset = offsetof(InstanceData, jointOffset),
      .format = gfx::PixelFormat::R32F }, // jointOffset
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
  CameraSystem::updateCameraUBO(*cam);

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

  // Gather & upload this frame's joint matrices once, before building the
  // instance data — each skinned instance needs the row its skeleton landed
  // on. Memoized, so this is a no-op if ShadowPass already ran it.
  const auto& frameSkinning = gatherAndUploadFrameSkinning(eManager);

  // Phase 1: Group every drawable node/primitive for instancing. Skinned
  // entities take part too: their differing skeletons are addressed through
  // the per-instance jointOffset rather than by splitting the draw.
  std::unordered_map<InstanceKey, std::vector<InstanceData>, InstanceKeyHash>
    instanceGroups;

  eManager.each<GraphicsComponent>([&](Entity entity,
                                       GraphicsComponent& gfxComp) {
    auto* posComp = eManager.getComponent<PositionComponent>(entity);
    auto* obj = gfxComp.m_grapObj.get();
    if (!obj) {
      return;
    }

    glm::mat4 entityModel =
      posComp ? glm::translate(glm::mat4(1.0f), posComp->position) *
                  glm::mat4_cast(posComp->rotation) *
                  glm::scale(glm::mat4(1.0f), posComp->scale)
              : glm::identity<glm::mat4>();

    // Node transforms come from this entity's own pose when it has one —
    // the GraphicsObject is shared between entities using the same model.
    auto* animComp = eManager.getComponent<AnimationComponent>(entity);
    SkeletonPose* pose =
      (animComp && animComp->pose.isBound()) ? &animComp->pose : nullptr;

    for (u32 nodeIdx = 0; nodeIdx < obj->p_numNodes; nodeIdx++) {
      if (obj->p_nodes[nodeIdx].mesh < 0) {
        continue;
      }

      const bool isSkinned = obj->p_nodes[nodeIdx].skin >= 0;

      InstanceData inst{};
      if (isSkinned) {
        // Skinning already encodes node-to-world.
        inst.modelMatrix = entityModel;
        inst.jointOffset =
          static_cast<float>(findJointOffset(frameSkinning, entity, nodeIdx));
      } else {
        glm::mat4 nodeMat = pose
                              ? pose->getMatrix(*obj, static_cast<i32>(nodeIdx))
                              : obj->getMatrix(nodeIdx);
        inst.modelMatrix = entityModel * nodeMat;
        inst.jointOffset = 0.0f;
      }

      Mesh& mesh = obj->p_meshes[obj->p_nodes[nodeIdx].mesh];
      for (u32 primIdx = 0; primIdx < mesh.numPrims; primIdx++) {
        instanceGroups[{ obj, nodeIdx, primIdx, isSkinned }].push_back(inst);
      }
    }
  });

  // Phase 2: Build contiguous instance buffer and draw groups
  static thread_local std::vector<InstanceData> allInstances;
  static thread_local std::vector<DrawGroup> drawGroups;
  allInstances.clear();
  drawGroups.clear();

  for (auto& [key, instances] : instanceGroups) {
    drawGroups.push_back({ key,
                           static_cast<u32>(allInstances.size()),
                           static_cast<u32>(instances.size()) });
    allInstances.insert(allInstances.end(), instances.begin(), instances.end());
  }

  // Upload all instance data
  if (!allInstances.empty()) {
    auto totalSize = static_cast<u32>(allInstances.size()) * kInstanceStride;

    // Resize buffer if needed
    if (static_cast<u32>(allInstances.size()) > m_instanceBufferCapacity) {
      device.destroyBuffer(m_instanceBuffer);
      m_instanceBufferCapacity = static_cast<u32>(allInstances.size()) * 2;
      gfx::BufferCreateInfo info{};
      info.size = static_cast<u64>(m_instanceBufferCapacity) * kInstanceStride;
      info.usage = gfx::BufferUsage::Vertex | gfx::BufferUsage::Dynamic;
      info.debugName = "GeometryInstanceBuffer";
      m_instanceBuffer = device.createBuffer(info);
    }

    device.updateBuffer(m_instanceBuffer, 0, allInstances.data(), totalSize);
  }

  // Phase 3: Issue instanced draw calls
  constexpr u32 kJointTexUnit = 5;
  bool lastSkinned = false;
  cmd->setUniform(m_isSkinnedLoc, 0);

  for (auto& group : drawGroups) {
    auto* obj = group.key.obj;
    Mesh& mesh = obj->p_meshes[obj->p_nodes[group.key.nodeIdx].mesh];
    Primitive& prim = mesh.m_primitives[group.key.primIdx];

    if (group.key.skinned != lastSkinned) {
      cmd->setUniform(m_isSkinnedLoc, group.key.skinned ? 1 : 0);
      lastSkinned = group.key.skinned;
    }
    if (group.key.skinned) {
      cmd->bindTextureByName(
        kJointTexUnit, "jointMats", resources.getLinearClampSampler());
    }

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
