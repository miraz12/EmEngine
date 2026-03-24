#pragma once

#include "GraphicsDevice.hpp"
#include "Singleton.hpp"
#include "UBOStructs.hpp"
#include <optional>
#include <string>
#include <unordered_map>

namespace gfx {

PixelFormat
gltfAccessorToPixelFormat(i32 componentType,
                          i32 numComponents,
                          bool isInteger = false);

PrimitiveTopology
gltfModeToTopology(i32 mode);

IndexType
gltfComponentTypeToIndexType(i32 componentType);

/// Bridge layer for incremental migration from direct GL to graphics
/// abstraction.
///
/// Migration strategy:
/// - Phase 1: Use createTexture/createFramebuffer - old passes still work via
/// string names
/// - Phase 2: Migrate passes to use handles directly, remove string lookups
/// - Phase 3: Remove old managers entirely
class RenderResources : public Singleton<RenderResources>
{
  friend class Singleton<RenderResources>;

public:
  /// Initialize shared resources (call after GraphicsDevice::initialize)
  bool initialize();

  void shutdown();

  TextureId createTexture2D(const std::string& name,
                            const TextureCreateInfo& info);

  TextureId createTexture2DArray(const std::string& name,
                                 const TextureCreateInfo& info);

  TextureId createTextureCube(const std::string& name,
                              const TextureCreateInfo& info);

  [[nodiscard]] TextureId getTexture(const std::string& name) const;

  [[nodiscard]] bool hasTexture(const std::string& name) const;

  void destroyTexture(const std::string& name);

  /// Recreate a texture with new dimensions (for viewport resize).
  /// Preserves the same handle but creates new GL storage.
  TextureId recreateTexture2D(const std::string& name,
                              u32 newWidth,
                              u32 newHeight);

  /// Create a data/lookup texture with NEAREST filtering and mutable storage.
  /// Used for textures that hold data values (joint matrices, LUTs, etc.)
  /// Returns the managed TextureId handle.
  TextureId createDataTexture(const std::string& name, PixelFormat format);

  /// Update a data texture's contents (mutable storage).
  /// If reallocate is true or dimensions changed, reallocates via glTexImage2D.
  /// Otherwise uses glTexSubImage2D for faster updates.
  void updateDataTexture(const std::string& name,
                         u32 width,
                         u32 height,
                         const void* data,
                         bool reallocate = false);

  FramebufferId createFramebuffer(const std::string& name,
                                  const FramebufferCreateInfo& info);

  /// Create a bare framebuffer (no attachments, for dynamic attachment)
  FramebufferId createBareFramebuffer(const std::string& name);

  [[nodiscard]] FramebufferId getFramebuffer(const std::string& name) const;
  [[nodiscard]] bool hasFramebuffer(const std::string& name) const;
  void destroyFramebuffer(const std::string& name);
  [[nodiscard]] bool isFramebufferComplete(const std::string& name) const;

  /// For cubemaps: layer specifies face (0=+X, 1=-X, 2=+Y, 3=-Y, 4=+Z, 5=-Z)
  void setFramebufferAttachment(const std::string& fboName,
                                u32 attachmentIndex,
                                const std::string& textureName,
                                u32 mipLevel = 0,
                                u32 layer = 0);

  void setFramebufferDepthAttachment(const std::string& fboName,
                                     const std::string& textureName,
                                     u32 mipLevel = 0,
                                     u32 layer = 0);

  /// @param attachmentType Depth, Stencil, or DepthStencil
  void setFramebufferRenderbuffer(const std::string& fboName,
                                  RenderbufferAttachment attachmentType,
                                  const std::string& rboName);

  /// Empty attachments = depth-only.
  void setDrawBuffers(const std::string& fboName,
                      std::span<const u32> attachments);

  RenderbufferId createRenderbuffer(const std::string& name,
                                    const RenderbufferCreateInfo& info);
  [[nodiscard]] RenderbufferId getRenderbuffer(const std::string& name) const;
  [[nodiscard]] bool hasRenderbuffer(const std::string& name) const;
  void destroyRenderbuffer(const std::string& name);
  void resizeRenderbuffer(const std::string& name, u32 newWidth, u32 newHeight);
  [[nodiscard]] u32 getNativeRenderbufferHandle(const std::string& name) const;

  BufferId createUniformBuffer(const std::string& name,
                               u64 size,
                               u32 bindingPoint);
  [[nodiscard]] BufferId getUniformBuffer(const std::string& name) const;
  [[nodiscard]] bool hasUniformBuffer(const std::string& name) const;
  void destroyUniformBuffer(const std::string& name);
  void updateUniformBuffer(const std::string& name, const void* data, u64 size);
  [[nodiscard]] u32 getNativeBufferHandle(const std::string& name) const;

  void createStandardUBOs();

  /// All getXxxUBO() accessors require calling the corresponding flushXxxUBO()
  /// after modification.
  [[nodiscard]] CameraUBO& getCameraUBO() { return m_cameraUBOData; }
  [[nodiscard]] LightingUBO& getLightingUBO() { return m_lightingUBOData; }
  [[nodiscard]] MaterialUBO& getMaterialUBO() { return m_materialUBOData; }
  [[nodiscard]] TransformUBO& getTransformUBO() { return m_transformUBOData; }
  [[nodiscard]] PostProcessUBO& getPostProcessUBO()
  {
    return m_postProcessUBOData;
  }

  void flushCameraUBO();
  void flushLightingUBO();
  void flushMaterialUBO();
  void flushTransformUBO();
  void flushPostProcessUBO();

  /// Bind a shader's uniform block to a standard UBO binding point.
  /// Call this once per shader that uses UBOs.
  /// @param program Shader program handle
  /// @param blockName Name of the uniform block in the shader
  /// @param binding Standard UBO binding point
  void bindShaderUniformBlock(ShaderId program,
                              const char* blockName,
                              UBOBinding binding);

  SamplerId createSampler(const std::string& name,
                          const SamplerCreateInfo& info);
  [[nodiscard]] SamplerId getSampler(const std::string& name) const;
  [[nodiscard]] SamplerId getLinearClampSampler() const
  {
    return m_linearClamp;
  }
  /// Get sampler with linear mipmap filtering and clamp wrap (for IBL cubemaps)
  [[nodiscard]] SamplerId getLinearMipmapClampSampler() const
  {
    return m_linearMipmapClamp;
  }
  [[nodiscard]] SamplerId getNearestClampSampler() const
  {
    return m_nearestClamp;
  }
  [[nodiscard]] SamplerId getLinearRepeatSampler() const
  {
    return m_linearRepeat;
  }
  [[nodiscard]] SamplerId getShadowSampler() const { return m_shadowSampler; }

  ShaderId loadShaderProgram(const std::string& name,
                             const std::string& vertPath,
                             const std::string& fragPath);
  [[nodiscard]] ShaderId getShaderProgram(const std::string& name) const;

  PipelineId createPipeline(const std::string& name,
                            const PipelineCreateInfo& info);
  [[nodiscard]] PipelineId getPipeline(const std::string& name) const;

  /// Layout: vec3 position, vec2 texcoord (interleaved, 5 floats per vertex)
  [[nodiscard]] BufferId getQuadVertexBuffer() const { return m_quadVBO; }

  /// Creates pipeline on first use with standard quad vertex layout
  [[nodiscard]] PipelineId getQuadPipeline(ShaderId shader);

  /// Layout: vec3 position, vec3 normal, vec2 texcoord (interleaved)
  [[nodiscard]] BufferId getCubeVertexBuffer() const { return m_cubeVBO; }

  static constexpr u32 kQuadVertexCount = 4;
  static constexpr u32 kCubeVertexCount = 36;

  /// Caller must bind shader and set uniforms before calling.
  void renderQuad();

  /// Caller must bind shader and set uniforms before calling.
  void renderCube();

  // Native handle access (for gradual migration)
  [[nodiscard]] u32 getNativeTextureHandle(const std::string& name) const;
  [[nodiscard]] u32 getNativeFramebufferHandle(const std::string& name) const;
  [[nodiscard]] u32 getNativeShaderHandle(const std::string& name) const;

  /// Resize all screen-relative textures and framebuffers.
  /// Call this from FrameGraph::setViewport.
  void setViewport(u32 width, u32 height);

  [[nodiscard]] u32 getWidth() const { return m_width; }
  [[nodiscard]] u32 getHeight() const { return m_height; }

  void bindFramebuffer(const std::string& name);
  void bindDefaultFramebuffer();
  void setViewportRect(i32 x, i32 y, u32 width, u32 height);
  void setScissorRect(i32 x, i32 y, u32 width, u32 height);

  /// nullopt = clear all attachments.
  void clearColor(std::optional<u32> attachment, const glm::vec4& color);
  void clearColor(std::optional<u32> attachment,
                  float r,
                  float g,
                  float b,
                  float a);
  void clearDepth(float depth = 1.0f);
  void clearStencil(i32 value = 0);
  void clearDepthStencil(float depth = 1.0f, i32 stencil = 0);

  void setDepthTest(bool enable);
  void setDepthFunc(CompareOp op);
  void setDepthWrite(bool enable);
  void setCullMode(CullMode mode);
  void setFrontFace(FrontFace face);
  void setBlendEnabled(bool enable);
  void setBlendFunc(BlendFactor srcColor,
                    BlendFactor dstColor,
                    BlendFactor srcAlpha,
                    BlendFactor dstAlpha);
  void setBlendFunc(BlendFactor src, BlendFactor dst);
  void setBlendEquation(BlendOp colorOp, BlendOp alphaOp);
  void setScissorTest(bool enable);

  /// Desktop GL only.
  void setSeamlessCubemap(bool enable);
  void setLineWidth(float width);
  /// Desktop GL only.
  void setLineSmoothing(bool enable);
  void setColorMask(bool red, bool green, bool blue, bool alpha);
  void setBlendColor(float red, float green, float blue, float alpha);

  /// 0-7 for color attachments, nullopt for GL_NONE.
  void setReadBuffer(std::optional<u32> attachment);

  void bindShaderProgram(const std::string& name);

  [[nodiscard]] i32 getUniformLocation(const std::string& shaderName,
                                       const char* uniformName) const;
  [[nodiscard]] i32 getAttribLocation(const std::string& shaderName,
                                      const char* attribName) const;

  void bindTexture(u32 unit, const std::string& name);
  void bindTexture(u32 unit, TextureId texture);
  void bindSampler(u32 unit, const std::string& name);
  void bindSampler(u32 unit, SamplerId sampler);

  void blitFramebuffer(const std::string& srcName,
                       const std::string& dstName,
                       i32 srcX0,
                       i32 srcY0,
                       i32 srcX1,
                       i32 srcY1,
                       i32 dstX0,
                       i32 dstY0,
                       i32 dstX1,
                       i32 dstY1,
                       bool colorBit,
                       bool depthBit,
                       bool stencilBit,
                       bool linearFilter = false);

  struct GeometryCreateResult
  {
    VertexArrayId vao;
    BufferId vbo;
    BufferId ebo; // Invalid if not indexed
  };

  GeometryCreateResult createGeometry(
    const void* vertexData,
    u64 vertexDataSize,
    const void* indexData,
    u64 indexDataSize,
    std::span<const VertexBinding> bindings,
    std::span<const VertexAttribute> attributes,
    PrimitiveTopology topology,
    u32 vertexCount,
    const char* debugName = nullptr);

private:
  RenderResources() = default;
  ~RenderResources();

  RenderResources(const RenderResources&) = delete;
  RenderResources& operator=(const RenderResources&) = delete;

  void createSharedSamplers();
  void createSharedGeometry();
  void createSharedDataTextures();
  std::string readShaderFile(const std::string& path);

  struct TextureEntry
  {
    TextureId handle;
    TextureCreateInfo info;
  };

  struct RenderbufferEntry
  {
    RenderbufferId handle;
    RenderbufferCreateInfo info;
  };

  struct UniformBufferEntry
  {
    BufferId handle;
    u64 size;
    u32 bindingPoint;
  };

  std::unordered_map<std::string, TextureEntry> m_textures;
  std::unordered_map<std::string, TextureId> m_dataTextures;
  std::unordered_map<std::string, FramebufferId> m_framebuffers;
  std::unordered_map<std::string, RenderbufferEntry> m_renderbuffers;
  std::unordered_map<std::string, UniformBufferEntry> m_uniformBuffers;
  std::unordered_map<std::string, SamplerId> m_samplers;
  std::unordered_map<std::string, ShaderId> m_shaders;
  std::unordered_map<std::string, PipelineId> m_pipelines;

  std::unordered_map<ShaderId, PipelineId> m_quadPipelines;

  SamplerId m_linearClamp;
  SamplerId m_nearestClamp;
  SamplerId m_linearRepeat;
  SamplerId m_shadowSampler;
  SamplerId m_linearMipmapClamp; // For IBL cubemaps that need mipmap access

  BufferId m_quadVBO;
  BufferId m_cubeVBO;
  VertexArrayId m_quadVAO; // Cached VAO for quad rendering
  VertexArrayId m_cubeVAO; // Cached VAO for cube rendering

  u32 m_width{ 800 };
  u32 m_height{ 600 };

  // GPU-side
  BufferId m_cameraUBO;
  BufferId m_lightingUBO;
  BufferId m_materialUBO;
  BufferId m_transformUBO;
  BufferId m_postProcessUBO;

  // CPU-side staging
  CameraUBO m_cameraUBOData{};
  LightingUBO m_lightingUBOData{};
  MaterialUBO m_materialUBOData{};
  TransformUBO m_transformUBOData{};
  PostProcessUBO m_postProcessUBOData{};

  bool m_initialized{ false };
};

} // namespace gfx
