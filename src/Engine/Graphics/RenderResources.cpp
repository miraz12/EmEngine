#include "RenderResources.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace gfx {

// tinygltf component type constants (from tiny_gltf.h)
constexpr i32 GLTF_UNSIGNED_BYTE = 5121;
constexpr i32 GLTF_UNSIGNED_SHORT = 5123;
constexpr i32 GLTF_UNSIGNED_INT = 5125;
constexpr i32 GLTF_FLOAT = 5126;

PixelFormat
gltfAccessorToPixelFormat(i32 componentType, i32 numComponents, bool isInteger)
{
  // For integer attributes (like JOINTS_0), use integer formats
  if (isInteger) {
    switch (componentType) {
      case GLTF_UNSIGNED_BYTE:
        switch (numComponents) {
          case 1:
            return PixelFormat::R8UI;
          case 2:
            return PixelFormat::RG8UI;
          case 3:
            return PixelFormat::RGB8UI;
          case 4:
            return PixelFormat::RGBA8UI;
        }
        break;
      case GLTF_UNSIGNED_SHORT:
        switch (numComponents) {
          case 1:
            return PixelFormat::R16UI;
          case 2:
            return PixelFormat::RG16UI;
          case 3:
            return PixelFormat::RGB16UI;
          case 4:
            return PixelFormat::RGBA16UI;
        }
        break;
    }
    return PixelFormat::Unknown;
  }

  // For normalized/float attributes
  switch (componentType) {
    case GLTF_UNSIGNED_BYTE:
      // Normalized 8-bit values (texcoords, weights, etc.)
      switch (numComponents) {
        case 1:
          return PixelFormat::R8;
        case 2:
          return PixelFormat::RG8;
        case 3:
          return PixelFormat::RGB8;
        case 4:
          return PixelFormat::RGBA8;
      }
      break;
    case GLTF_FLOAT:
      switch (numComponents) {
        case 1:
          return PixelFormat::R32F;
        case 2:
          return PixelFormat::RG32F;
        case 3:
          return PixelFormat::RGB32F;
        case 4:
          return PixelFormat::RGBA32F;
      }
      break;
  }
  return PixelFormat::Unknown;
}

PrimitiveTopology
gltfModeToTopology(i32 mode)
{
  // tinygltf mode constants
  constexpr i32 GLTF_POINTS = 0;
  constexpr i32 GLTF_LINES = 1;
  constexpr i32 GLTF_LINE_STRIP = 3;
  constexpr i32 GLTF_TRIANGLES = 4;
  constexpr i32 GLTF_TRIANGLE_STRIP = 5;
  constexpr i32 GLTF_TRIANGLE_FAN = 6;

  switch (mode) {
    case GLTF_POINTS:
      return PrimitiveTopology::Points;
    case GLTF_LINES:
      return PrimitiveTopology::Lines;
    case GLTF_LINE_STRIP:
      return PrimitiveTopology::LineStrip;
    case GLTF_TRIANGLES:
      return PrimitiveTopology::Triangles;
    case GLTF_TRIANGLE_STRIP:
      return PrimitiveTopology::TriangleStrip;
    case GLTF_TRIANGLE_FAN:
      return PrimitiveTopology::TriangleFan;
    default:
      return PrimitiveTopology::Triangles;
  }
}

IndexType
gltfComponentTypeToIndexType(i32 componentType)
{
  switch (componentType) {
    case GLTF_UNSIGNED_INT:
      return IndexType::U32;
    case GLTF_UNSIGNED_SHORT:
    default:
      return IndexType::U16;
  }
}

RenderResources::~RenderResources()
{
  if (m_initialized) {
    shutdown();
  }
}

bool
RenderResources::initialize()
{
  if (m_initialized) {
    return true;
  }

  createSharedSamplers();
  createSharedGeometry();
  createSharedDataTextures();
  createStandardUBOs();

  m_initialized = true;
  return true;
}

void
RenderResources::shutdown()
{
  if (!m_initialized) {
    return;
  }

  auto& device = GraphicsDevice::getInstance();

  for (auto& [name, pipeline] : m_pipelines) {
    device.destroyPipeline(pipeline);
  }
  m_pipelines.clear();

  for (auto& [shaderValue, pipeline] : m_quadPipelines) {
    device.destroyPipeline(pipeline);
  }
  m_quadPipelines.clear();

  for (auto& [name, shader] : m_shaders) {
    device.destroyShader(shader);
  }
  m_shaders.clear();

  for (auto& [name, fb] : m_framebuffers) {
    device.destroyFramebuffer(fb);
  }
  m_framebuffers.clear();

  for (auto& [name, entry] : m_renderbuffers) {
    device.destroyRenderbuffer(entry.handle);
  }
  m_renderbuffers.clear();

  for (auto& [name, entry] : m_uniformBuffers) {
    device.destroyBuffer(entry.handle);
  }
  m_uniformBuffers.clear();

  for (auto& [name, entry] : m_textures) {
    device.destroyTexture(entry.handle);
  }
  m_textures.clear();

  for (auto& [name, sampler] : m_samplers) {
    device.destroySampler(sampler);
  }
  m_samplers.clear();

  if (m_linearClamp.isValid())
    device.destroySampler(m_linearClamp);
  if (m_linearMipmapClamp.isValid())
    device.destroySampler(m_linearMipmapClamp);
  if (m_nearestClamp.isValid())
    device.destroySampler(m_nearestClamp);
  if (m_linearRepeat.isValid())
    device.destroySampler(m_linearRepeat);
  if (m_shadowSampler.isValid())
    device.destroySampler(m_shadowSampler);

  for (auto& [name, handle] : m_dataTextures) {
    if (handle.isValid())
      device.destroyTexture(handle);
  }
  m_dataTextures.clear();

  if (m_quadVBO.isValid())
    device.destroyBuffer(m_quadVBO);
  if (m_cubeVBO.isValid())
    device.destroyBuffer(m_cubeVBO);

  if (m_cameraUBO.isValid())
    device.destroyBuffer(m_cameraUBO);
  if (m_lightingUBO.isValid())
    device.destroyBuffer(m_lightingUBO);
  if (m_materialUBO.isValid())
    device.destroyBuffer(m_materialUBO);
  if (m_transformUBO.isValid())
    device.destroyBuffer(m_transformUBO);
  if (m_postProcessUBO.isValid())
    device.destroyBuffer(m_postProcessUBO);

  if (m_quadVAO.isValid()) {
    device.destroyVertexArray(m_quadVAO);
    m_quadVAO = {};
  }
  if (m_cubeVAO.isValid()) {
    device.destroyVertexArray(m_cubeVAO);
    m_cubeVAO = {};
  }

  m_initialized = false;
}

void
RenderResources::createSharedSamplers()
{
  auto& device = GraphicsDevice::getInstance();

  // Linear filtering, clamp to edge (most common for post-processing)
  {
    SamplerCreateInfo info{};
    info.minFilter = FilterMode::Linear;
    info.magFilter = FilterMode::Linear;
    info.wrapU = WrapMode::ClampToEdge;
    info.wrapV = WrapMode::ClampToEdge;
    info.wrapW = WrapMode::ClampToEdge;
    info.debugName = "LinearClamp";
    m_linearClamp = device.createSampler(info);
  }

  // Linear filtering with mipmaps, clamp to edge (for IBL cubemaps that need
  // mip level access via textureLod)
  {
    SamplerCreateInfo info{};
    info.minFilter = FilterMode::LinearMipmapLinear;
    info.magFilter = FilterMode::Linear;
    info.wrapU = WrapMode::ClampToEdge;
    info.wrapV = WrapMode::ClampToEdge;
    info.wrapW = WrapMode::ClampToEdge;
    info.debugName = "LinearMipmapClamp";
    m_linearMipmapClamp = device.createSampler(info);
  }

  {
    SamplerCreateInfo info{};
    info.minFilter = FilterMode::Nearest;
    info.magFilter = FilterMode::Nearest;
    info.wrapU = WrapMode::ClampToEdge;
    info.wrapV = WrapMode::ClampToEdge;
    info.wrapW = WrapMode::ClampToEdge;
    info.debugName = "NearestClamp";
    m_nearestClamp = device.createSampler(info);
  }

  // Linear filtering, repeat (for tiled textures)
  {
    SamplerCreateInfo info{};
    info.minFilter = FilterMode::LinearMipmapLinear;
    info.magFilter = FilterMode::Linear;
    info.wrapU = WrapMode::Repeat;
    info.wrapV = WrapMode::Repeat;
    info.wrapW = WrapMode::Repeat;
    info.debugName = "LinearRepeat";
    m_linearRepeat = device.createSampler(info);
  }

  // Shadow sampler (comparison mode for shadow mapping)
  {
    SamplerCreateInfo info{};
    info.minFilter = FilterMode::Linear;
    info.magFilter = FilterMode::Linear;
    info.wrapU = WrapMode::ClampToEdge;
    info.wrapV = WrapMode::ClampToEdge;
    info.wrapW = WrapMode::ClampToEdge;
    info.compareEnable = true;
    info.compareOp = CompareOp::LessEqual;
    info.debugName = "ShadowSampler";
    m_shadowSampler = device.createSampler(info);
  }
}

void
RenderResources::createSharedDataTextures()
{
  // Joint matrices texture - shared by ShadowPass and GeometryPass
  // Used for skeletal animation, holds mat4 data encoded in a texture
  // Actual storage allocated when joint data is uploaded via glTexImage2D
  createDataTexture("jointMats", PixelFormat::RGBA32F);

  // Default textures for materials - 1x1 pixel fallbacks
  // "black_default" - used for missing base color, metallic/roughness,
  // emissive, occlusion
  {
    std::array<u8, 4> blackPixel = { 0, 0, 0, 255 };
    TextureCreateInfo info{};
    info.width = 1;
    info.height = 1;
    info.format = PixelFormat::RGBA8;
    info.mipLevels = 1;
    info.initialData = blackPixel.data();
    info.debugName = "black_default";
    createTexture2D("black_default", info);
  }

  // "metallic_roughness_default" - glTF PBR defaults: roughness=1.0,
  // metallic=0.0 glTF format: R=unused, G=roughness, B=metallic, A=unused
  {
    std::array<u8, 4> defaultMR = {
      255, 255, 0, 255
    }; // G=255 (rough), B=0 (non-metallic)
    TextureCreateInfo info{};
    info.width = 1;
    info.height = 1;
    info.format = PixelFormat::RGBA8;
    info.mipLevels = 1;
    info.initialData = defaultMR.data();
    info.debugName = "metallic_roughness_default";
    createTexture2D("metallic_roughness_default", info);
  }
}

void
RenderResources::createSharedGeometry()
{
  auto& device = GraphicsDevice::getInstance();

  // Fullscreen quad (triangle strip, CCW)
  // Layout: vec3 position, vec2 texcoord
  // clang-format off
    float quadVertices[] = {
        // positions         // texCoords
        -1.0f,  1.0f, 0.0f,  0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f,  0.0f, 0.0f,
         1.0f,  1.0f, 0.0f,  1.0f, 1.0f,
         1.0f, -1.0f, 0.0f,  1.0f, 0.0f,
    };
  // clang-format on

  BufferCreateInfo quadInfo{};
  quadInfo.size = sizeof(quadVertices);
  quadInfo.usage = BufferUsage::Vertex;
  quadInfo.initialData = quadVertices;
  quadInfo.debugName = "SharedQuadVBO";
  m_quadVBO = device.createBuffer(quadInfo);

  // Unit cube (triangles, CCW front faces)
  // Layout: vec3 position, vec3 normal, vec2 texcoord
  // clang-format off
    float cubeVertices[] = {
        // back face
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
        // front face
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
        // left face
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
        // right face
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
        // bottom face
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
         1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
         1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
        -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
        // top face
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
         1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
         1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
        -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
    };
  // clang-format on

  BufferCreateInfo cubeInfo{};
  cubeInfo.size = sizeof(cubeVertices);
  cubeInfo.usage = BufferUsage::Vertex;
  cubeInfo.initialData = cubeVertices;
  cubeInfo.debugName = "SharedCubeVBO";
  m_cubeVBO = device.createBuffer(cubeInfo);
}

std::string
RenderResources::readShaderFile(const std::string& path)
{
  std::ifstream file(path);
  if (!file.is_open()) {
    std::cerr << "Failed to open shader file: " << path << std::endl;
    return "";
  }

  std::stringstream buffer;
  buffer << file.rdbuf();
  return buffer.str();
}

TextureId
RenderResources::createTexture2D(const std::string& name,
                                 const TextureCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  TextureCreateInfo modifiedInfo = info;
  modifiedInfo.type = TextureType::Texture2D;

  TextureId handle = device.createTexture(modifiedInfo);
  if (!handle.isValid()) {
    std::cerr << "Failed to create texture: " << name << std::endl;
    return TextureId{};
  }

  // Store for potential recreation on resize
  m_textures[name] = { handle, modifiedInfo };

  return handle;
}

TextureId
RenderResources::createTexture2DArray(const std::string& name,
                                      const TextureCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  TextureCreateInfo modifiedInfo = info;
  modifiedInfo.type = TextureType::Texture2DArray;

  TextureId handle = device.createTexture(modifiedInfo);
  if (!handle.isValid()) {
    std::cerr << "Failed to create texture array: " << name << std::endl;
    return TextureId{};
  }

  m_textures[name] = { handle, modifiedInfo };

  return handle;
}

TextureId
RenderResources::createTextureCube(const std::string& name,
                                   const TextureCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  TextureCreateInfo modifiedInfo = info;
  modifiedInfo.type = TextureType::TextureCube;

  TextureId handle = device.createTexture(modifiedInfo);
  if (!handle.isValid()) {
    std::cerr << "Failed to create cubemap texture: " << name << std::endl;
    return TextureId{};
  }

  m_textures[name] = { handle, modifiedInfo };

  return handle;
}

TextureId
RenderResources::getTexture(const std::string& name) const
{
  auto it = m_textures.find(name);
  return (it != m_textures.end()) ? it->second.handle : TextureId{};
}

bool
RenderResources::hasTexture(const std::string& name) const
{
  return m_textures.find(name) != m_textures.end();
}

void
RenderResources::destroyTexture(const std::string& name)
{
  auto it = m_textures.find(name);
  if (it == m_textures.end()) {
    return;
  }

  GraphicsDevice::getInstance().destroyTexture(it->second.handle);
  m_textures.erase(it);
}

TextureId
RenderResources::createDataTexture(const std::string& name, PixelFormat format)
{
  auto& device = GraphicsDevice::getInstance();

  TextureCreateInfo info{};
  info.type = TextureType::Texture2D;
  info.format = format;
  info.width = 1;
  info.height = 1;
  info.mipLevels = 1;
  info.usage = TextureUsage::Sampled;
  info.storageMode = TextureStorageMode::Mutable;
  info.debugName = name.c_str();

  TextureId handle = device.createTexture(info);
  m_dataTextures[name] = handle;
  return handle;
}

void
RenderResources::updateDataTexture(const std::string& name,
                                   u32 width,
                                   u32 height,
                                   const void* data,
                                   bool reallocate)
{
  auto it = m_dataTextures.find(name);
  if (it == m_dataTextures.end()) {
    std::cerr << "Cannot update non-existent data texture: " << name
              << std::endl;
    return;
  }

  auto& device = GraphicsDevice::getInstance();

  // Always use resizeTexture (glTexImage2D) for simplicity and correctness.
  // This handles both reallocation and in-place update cases since mutable
  // textures use glTexImage2D which can change dimensions.
  device.resizeTexture(it->second, width, height, data);
}

TextureId
RenderResources::recreateTexture2D(const std::string& name,
                                   u32 newWidth,
                                   u32 newHeight)
{
  auto it = m_textures.find(name);
  if (it == m_textures.end()) {
    std::cerr << "Cannot recreate non-existent texture: " << name << std::endl;
    return TextureId{};
  }

  auto& device = GraphicsDevice::getInstance();

  device.destroyTexture(it->second.handle);

  TextureCreateInfo newInfo = it->second.info;
  newInfo.width = newWidth;
  newInfo.height = newHeight;
  newInfo.initialData = nullptr; // Render targets don't need initial data

  TextureId newHandle = device.createTexture(newInfo);
  if (!newHandle.isValid()) {
    std::cerr << "Failed to recreate texture: " << name << std::endl;
    m_textures.erase(it);
    return TextureId{};
  }

  it->second.handle = newHandle;
  it->second.info = newInfo;

  return newHandle;
}

FramebufferId
RenderResources::createFramebuffer(const std::string& name,
                                   const FramebufferCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  FramebufferId handle = device.createFramebuffer(info);
  if (!handle.isValid()) {
    std::cerr << "Failed to create framebuffer: " << name << std::endl;
    return FramebufferId{};
  }

  m_framebuffers[name] = handle;
  return handle;
}

FramebufferId
RenderResources::createBareFramebuffer(const std::string& name)
{
  auto& device = GraphicsDevice::getInstance();

  // Create with no attachments - they'll be managed separately
  FramebufferCreateInfo info{};
  info.debugName = name.c_str();

  FramebufferId handle = device.createFramebuffer(info);
  if (!handle.isValid()) {
    std::cerr << "Failed to create bare framebuffer: " << name << std::endl;
    return FramebufferId{};
  }

  m_framebuffers[name] = handle;
  return handle;
}

FramebufferId
RenderResources::getFramebuffer(const std::string& name) const
{
  auto it = m_framebuffers.find(name);
  return (it != m_framebuffers.end()) ? it->second : FramebufferId{};
}

bool
RenderResources::hasFramebuffer(const std::string& name) const
{
  return m_framebuffers.find(name) != m_framebuffers.end();
}

void
RenderResources::destroyFramebuffer(const std::string& name)
{
  auto it = m_framebuffers.find(name);
  if (it == m_framebuffers.end()) {
    return;
  }

  GraphicsDevice::getInstance().destroyFramebuffer(it->second);
  m_framebuffers.erase(it);
}

bool
RenderResources::isFramebufferComplete(const std::string& name) const
{
  auto it = m_framebuffers.find(name);
  if (it == m_framebuffers.end()) {
    return false;
  }

  return GraphicsDevice::getInstance().isFramebufferComplete(it->second);
}

void
RenderResources::setFramebufferAttachment(const std::string& fboName,
                                          u32 attachmentIndex,
                                          const std::string& textureName,
                                          u32 mipLevel,
                                          u32 layer)
{
  FramebufferId fbo = getFramebuffer(fboName);
  TextureId tex = getTexture(textureName);
  if (!fbo.isValid() || !tex.isValid()) {
    std::cerr << "setFramebufferAttachment: invalid fbo '" << fboName
              << "' or texture '" << textureName << "'" << std::endl;
    return;
  }
  GraphicsDevice::getInstance().setFramebufferAttachment(
    fbo, attachmentIndex, tex, mipLevel, layer);
}

void
RenderResources::setFramebufferDepthAttachment(const std::string& fboName,
                                               const std::string& textureName,
                                               u32 mipLevel,
                                               u32 layer)
{
  FramebufferId fbo = getFramebuffer(fboName);
  TextureId tex = getTexture(textureName);
  if (!fbo.isValid() || !tex.isValid()) {
    std::cerr << "setFramebufferDepthAttachment: invalid fbo '" << fboName
              << "' or texture '" << textureName << "'" << std::endl;
    return;
  }
  GraphicsDevice::getInstance().setFramebufferDepthAttachment(
    fbo, tex, mipLevel, layer);
}

void
RenderResources::setFramebufferRenderbuffer(
  const std::string& fboName,
  RenderbufferAttachment attachmentType,
  const std::string& rboName)
{
  FramebufferId fbo = getFramebuffer(fboName);
  RenderbufferId rbo = getRenderbuffer(rboName);
  if (!fbo.isValid() || !rbo.isValid()) {
    std::cerr << "setFramebufferRenderbuffer: invalid fbo '" << fboName
              << "' or renderbuffer '" << rboName << "'" << std::endl;
    return;
  }
  GraphicsDevice::getInstance().setFramebufferRenderbuffer(
    fbo, attachmentType, rbo);
}

void
RenderResources::setDrawBuffers(const std::string& fboName,
                                std::span<const u32> attachments)
{
  FramebufferId fbo = getFramebuffer(fboName);
  if (!fbo.isValid()) {
    std::cerr << "setDrawBuffers: invalid fbo '" << fboName << "'" << std::endl;
    return;
  }
  GraphicsDevice::getInstance().setDrawBuffers(fbo, attachments);
}

RenderbufferId
RenderResources::createRenderbuffer(const std::string& name,
                                    const RenderbufferCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  RenderbufferId handle = device.createRenderbuffer(info);
  if (!handle.isValid()) {
    std::cerr << "Failed to create renderbuffer: " << name << std::endl;
    return RenderbufferId{};
  }

  m_renderbuffers[name] = { handle, info };
  return handle;
}

RenderbufferId
RenderResources::getRenderbuffer(const std::string& name) const
{
  auto it = m_renderbuffers.find(name);
  return (it != m_renderbuffers.end()) ? it->second.handle : RenderbufferId{};
}

bool
RenderResources::hasRenderbuffer(const std::string& name) const
{
  return m_renderbuffers.find(name) != m_renderbuffers.end();
}

void
RenderResources::destroyRenderbuffer(const std::string& name)
{
  auto it = m_renderbuffers.find(name);
  if (it == m_renderbuffers.end()) {
    return;
  }

  GraphicsDevice::getInstance().destroyRenderbuffer(it->second.handle);
  m_renderbuffers.erase(it);
}

void
RenderResources::resizeRenderbuffer(const std::string& name,
                                    u32 newWidth,
                                    u32 newHeight)
{
  auto it = m_renderbuffers.find(name);
  if (it == m_renderbuffers.end()) {
    std::cerr << "Cannot resize non-existent renderbuffer: " << name
              << std::endl;
    return;
  }

  GraphicsDevice::getInstance().resizeRenderbuffer(
    it->second.handle, newWidth, newHeight);

  it->second.info.width = newWidth;
  it->second.info.height = newHeight;
}

u32
RenderResources::getNativeRenderbufferHandle(const std::string& name) const
{
  auto it = m_renderbuffers.find(name);
  if (it == m_renderbuffers.end()) {
    return 0;
  }
  return GraphicsDevice::getInstance().getNativeHandle(it->second.handle);
}

BufferId
RenderResources::createUniformBuffer(const std::string& name,
                                     u64 size,
                                     u32 bindingPoint)
{
  auto& device = GraphicsDevice::getInstance();

  BufferCreateInfo info{};
  info.size = size;
  info.usage = BufferUsage::Uniform;
  info.debugName = name.c_str();

  BufferId handle = device.createBuffer(info);
  if (!handle.isValid()) {
    std::cerr << "Failed to create uniform buffer: " << name << std::endl;
    return BufferId{};
  }

  device.bindUniformBuffer(bindingPoint, handle, 0, size);

  m_uniformBuffers[name] = { handle, size, bindingPoint };
  return handle;
}

BufferId
RenderResources::getUniformBuffer(const std::string& name) const
{
  auto it = m_uniformBuffers.find(name);
  return (it != m_uniformBuffers.end()) ? it->second.handle : BufferId{};
}

bool
RenderResources::hasUniformBuffer(const std::string& name) const
{
  return m_uniformBuffers.find(name) != m_uniformBuffers.end();
}

void
RenderResources::destroyUniformBuffer(const std::string& name)
{
  auto it = m_uniformBuffers.find(name);
  if (it == m_uniformBuffers.end()) {
    return;
  }

  GraphicsDevice::getInstance().destroyBuffer(it->second.handle);
  m_uniformBuffers.erase(it);
}

void
RenderResources::updateUniformBuffer(const std::string& name,
                                     const void* data,
                                     u64 size)
{
  auto it = m_uniformBuffers.find(name);
  if (it == m_uniformBuffers.end()) {
    std::cerr << "Cannot update non-existent uniform buffer: " << name
              << std::endl;
    return;
  }

  GraphicsDevice::getInstance().updateBuffer(it->second.handle, 0, data, size);
}

u32
RenderResources::getNativeBufferHandle(const std::string& name) const
{
  auto it = m_uniformBuffers.find(name);
  if (it == m_uniformBuffers.end()) {
    return 0;
  }
  return GraphicsDevice::getInstance().getNativeHandle(it->second.handle);
}

void
RenderResources::createStandardUBOs()
{
  auto& device = GraphicsDevice::getInstance();

  {
    BufferCreateInfo info{};
    info.size = sizeof(CameraUBO);
    info.usage = BufferUsage::Uniform;
    info.debugName = "CameraUBO";
    m_cameraUBO = device.createBuffer(info);
    device.bindUniformBuffer(
      static_cast<u32>(UBOBinding::Camera), m_cameraUBO, 0, sizeof(CameraUBO));
  }

  {
    BufferCreateInfo info{};
    info.size = sizeof(LightingUBO);
    info.usage = BufferUsage::Uniform;
    info.debugName = "LightingUBO";
    m_lightingUBO = device.createBuffer(info);
    device.bindUniformBuffer(static_cast<u32>(UBOBinding::Lighting),
                             m_lightingUBO,
                             0,
                             sizeof(LightingUBO));
  }

  {
    BufferCreateInfo info{};
    info.size = sizeof(MaterialUBO);
    info.usage = BufferUsage::Uniform;
    info.debugName = "MaterialUBO";
    m_materialUBO = device.createBuffer(info);
    device.bindUniformBuffer(static_cast<u32>(UBOBinding::Material),
                             m_materialUBO,
                             0,
                             sizeof(MaterialUBO));
  }

  {
    BufferCreateInfo info{};
    info.size = sizeof(TransformUBO);
    info.usage = BufferUsage::Uniform;
    info.debugName = "TransformUBO";
    m_transformUBO = device.createBuffer(info);
    device.bindUniformBuffer(static_cast<u32>(UBOBinding::Transform),
                             m_transformUBO,
                             0,
                             sizeof(TransformUBO));
  }

  {
    BufferCreateInfo info{};
    info.size = sizeof(PostProcessUBO);
    info.usage = BufferUsage::Uniform;
    info.debugName = "PostProcessUBO";
    m_postProcessUBO = device.createBuffer(info);
    device.bindUniformBuffer(static_cast<u32>(UBOBinding::PostProcess),
                             m_postProcessUBO,
                             0,
                             sizeof(PostProcessUBO));
  }
}

void
RenderResources::flushCameraUBO()
{
  auto& device = GraphicsDevice::getInstance();
  device.updateBuffer(m_cameraUBO, 0, &m_cameraUBOData, sizeof(CameraUBO));
  // Ensure UBO is bound to binding point after update
  device.bindUniformBuffer(
    static_cast<u32>(UBOBinding::Camera), m_cameraUBO, 0, sizeof(CameraUBO));
}

void
RenderResources::flushLightingUBO()
{
  auto& device = GraphicsDevice::getInstance();
  device.updateBuffer(
    m_lightingUBO, 0, &m_lightingUBOData, sizeof(LightingUBO));
  // Ensure UBO is bound to binding point after update
  device.bindUniformBuffer(static_cast<u32>(UBOBinding::Lighting),
                           m_lightingUBO,
                           0,
                           sizeof(LightingUBO));
}

void
RenderResources::flushMaterialUBO()
{
  GraphicsDevice::getInstance().updateBuffer(
    m_materialUBO, 0, &m_materialUBOData, sizeof(MaterialUBO));
}

void
RenderResources::flushTransformUBO()
{
  GraphicsDevice::getInstance().updateBuffer(
    m_transformUBO, 0, &m_transformUBOData, sizeof(TransformUBO));
}

void
RenderResources::flushPostProcessUBO()
{
  auto& device = GraphicsDevice::getInstance();
  device.updateBuffer(
    m_postProcessUBO, 0, &m_postProcessUBOData, sizeof(PostProcessUBO));
  // Ensure UBO is bound to binding point after update
  device.bindUniformBuffer(static_cast<u32>(UBOBinding::PostProcess),
                           m_postProcessUBO,
                           0,
                           sizeof(PostProcessUBO));
}

void
RenderResources::bindShaderUniformBlock(ShaderId program,
                                        const char* blockName,
                                        UBOBinding binding)
{
  GraphicsDevice::getInstance().bindUniformBlock(
    program, blockName, static_cast<u32>(binding));
}

SamplerId
RenderResources::createSampler(const std::string& name,
                               const SamplerCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  SamplerId handle = device.createSampler(info);
  if (!handle.isValid()) {
    std::cerr << "Failed to create sampler: " << name << std::endl;
    return SamplerId{};
  }

  m_samplers[name] = handle;
  return handle;
}

SamplerId
RenderResources::getSampler(const std::string& name) const
{
  auto it = m_samplers.find(name);
  return (it != m_samplers.end()) ? it->second : SamplerId{};
}

ShaderId
RenderResources::loadShaderProgram(const std::string& name,
                                   const std::string& vertPath,
                                   const std::string& fragPath)
{
  auto& device = GraphicsDevice::getInstance();

  std::string vertSource = readShaderFile(vertPath);
  std::string fragSource = readShaderFile(fragPath);

  if (vertSource.empty() || fragSource.empty()) {
    std::cerr << "Failed to load shader files for: " << name << std::endl;
    return ShaderId{};
  }

  ShaderCreateInfo vertInfo{};
  vertInfo.stage = ShaderStage::Vertex;
  vertInfo.source = vertSource.c_str();
  vertInfo.sourceLength = vertSource.length();
  vertInfo.debugName = vertPath.c_str();

  ShaderId vertShader = device.createShader(vertInfo);
  if (!vertShader.isValid()) {
    std::cerr << "Failed to compile vertex shader: " << vertPath << std::endl;
    return ShaderId{};
  }

  ShaderCreateInfo fragInfo{};
  fragInfo.stage = ShaderStage::Fragment;
  fragInfo.source = fragSource.c_str();
  fragInfo.sourceLength = fragSource.length();
  fragInfo.debugName = fragPath.c_str();

  ShaderId fragShader = device.createShader(fragInfo);
  if (!fragShader.isValid()) {
    device.destroyShader(vertShader);
    std::cerr << "Failed to compile fragment shader: " << fragPath << std::endl;
    return ShaderId{};
  }

  ShaderProgramCreateInfo progInfo{};
  progInfo.vertexShader = vertShader;
  progInfo.fragmentShader = fragShader;
  progInfo.debugName = name.c_str();

  ShaderId program = device.createShaderProgram(progInfo);

  // Clean up individual shaders (program keeps them linked)
  device.destroyShader(vertShader);
  device.destroyShader(fragShader);

  if (!program.isValid()) {
    std::cerr << "Failed to link shader program: " << name << std::endl;
    return ShaderId{};
  }

  m_shaders[name] = program;
  return program;
}

ShaderId
RenderResources::getShaderProgram(const std::string& name) const
{
  auto it = m_shaders.find(name);
  return (it != m_shaders.end()) ? it->second : ShaderId{};
}

PipelineId
RenderResources::createPipeline(const std::string& name,
                                const PipelineCreateInfo& info)
{
  auto& device = GraphicsDevice::getInstance();

  PipelineId handle = device.createPipeline(info);
  if (!handle.isValid()) {
    std::cerr << "Failed to create pipeline: " << name << std::endl;
    return PipelineId{};
  }

  m_pipelines[name] = handle;
  return handle;
}

PipelineId
RenderResources::getPipeline(const std::string& name) const
{
  auto it = m_pipelines.find(name);
  return (it != m_pipelines.end()) ? it->second : PipelineId{};
}

PipelineId
RenderResources::getQuadPipeline(ShaderId shader)
{
  if (!shader.isValid()) {
    return PipelineId{};
  }

  // Check cache
  auto it = m_quadPipelines.find(shader);
  if (it != m_quadPipelines.end()) {
    return it->second;
  }

  // Create new pipeline for this shader
  // Quad layout: vec3 position (loc 0), vec2 texcoord (loc 1)
  std::array<VertexBinding, 1> bindings = { {
    { 0, 5 * sizeof(float), false } // binding 0, stride 20 bytes
  } };

  std::array<VertexAttribute, 2> attributes = { {
    { 0, 0, 0, PixelFormat::RGB32F }, // location 0, binding 0, offset 0
    { 1,
      0,
      3 * sizeof(float),
      PixelFormat::RG32F } // location 1, binding 0, offset 12
  } };

  PipelineCreateInfo info{};
  info.shaderProgram = shader;
  info.vertexBindings = bindings;
  info.vertexAttributes = attributes;
  info.topology = PrimitiveTopology::TriangleStrip;
  info.rasterizer.cullMode = CullMode::None; // Quads don't need culling
  info.depthStencil.depthTestEnable = false;
  info.depthStencil.depthWriteEnable = false;
  info.colorAttachmentCount = 1;
  info.debugName = "QuadPipeline";

  PipelineId pipeline = GraphicsDevice::getInstance().createPipeline(info);
  if (pipeline.isValid()) {
    m_quadPipelines[shader] = pipeline;
  }

  return pipeline;
}

u32
RenderResources::getNativeTextureHandle(const std::string& name) const
{
  auto it = m_textures.find(name);
  if (it == m_textures.end()) {
    return 0;
  }
  return GraphicsDevice::getInstance().getNativeHandle(it->second.handle);
}

u32
RenderResources::getNativeFramebufferHandle(const std::string& name) const
{
  auto it = m_framebuffers.find(name);
  if (it == m_framebuffers.end()) {
    return 0;
  }
  return GraphicsDevice::getInstance().getNativeHandle(it->second);
}

u32
RenderResources::getNativeShaderHandle(const std::string& name) const
{
  auto it = m_shaders.find(name);
  if (it == m_shaders.end()) {
    return 0;
  }
  return GraphicsDevice::getInstance().getNativeHandle(it->second);
}

void
RenderResources::setViewport(u32 width, u32 height)
{
  m_width = width;
  m_height = height;
  // Note: Individual passes are responsible for recreating their
  // screen-sized resources when viewport changes
}

void
RenderResources::renderQuad()
{
  auto& device = GraphicsDevice::getInstance();

  if (!m_quadVAO.isValid()) {
    // Quad layout: vec3 position (loc 0), vec2 texcoord (loc 1)
    std::array<VertexBinding, 1> bindings = {
      { { 0, 5 * sizeof(float), false } }
    };
    std::array<VertexAttribute, 2> attributes = { {
      { 0, 0, 0, PixelFormat::RGB32F },               // position
      { 1, 0, 3 * sizeof(float), PixelFormat::RG32F } // texcoord
    } };

    VertexArrayCreateInfo info{};
    info.vertexBindings = bindings;
    info.vertexAttributes = attributes;
    info.topology = PrimitiveTopology::TriangleStrip;
    info.vertexCount = kQuadVertexCount;
    info.debugName = "sharedQuadVAO";

    m_quadVAO = device.createVertexArray(info);
  }

  device.drawVertexArray(m_quadVAO, m_quadVBO);
}

void
RenderResources::renderCube()
{
  auto& device = GraphicsDevice::getInstance();

  if (!m_cubeVAO.isValid()) {
    // Cube layout: vec3 position (loc 0), vec3 normal (loc 1), vec2 texcoord
    // (loc 2)
    std::array<VertexBinding, 1> bindings = {
      { { 0, 8 * sizeof(float), false } }
    };
    std::array<VertexAttribute, 3> attributes = { {
      { 0, 0, 0, PixelFormat::RGB32F },                 // position
      { 1, 0, 3 * sizeof(float), PixelFormat::RGB32F }, // normal
      { 2, 0, 6 * sizeof(float), PixelFormat::RG32F }   // texcoord
    } };

    VertexArrayCreateInfo info{};
    info.vertexBindings = bindings;
    info.vertexAttributes = attributes;
    info.topology = PrimitiveTopology::Triangles;
    info.vertexCount = kCubeVertexCount;
    info.debugName = "sharedCubeVAO";

    m_cubeVAO = device.createVertexArray(info);
  }

  device.drawVertexArray(m_cubeVAO, m_cubeVBO);
}

void
RenderResources::bindFramebuffer(const std::string& name)
{
  auto& device = GraphicsDevice::getInstance();
  if (name.empty()) {
    device.bindFramebuffer(device.getDefaultFramebuffer());
  } else {
    FramebufferId fbo = getFramebuffer(name);
    device.bindFramebuffer(fbo);
  }
}

void
RenderResources::bindDefaultFramebuffer()
{
  auto& device = GraphicsDevice::getInstance();
  device.bindFramebuffer(device.getDefaultFramebuffer());
}

void
RenderResources::setViewportRect(i32 x, i32 y, u32 width, u32 height)
{
  GraphicsDevice::getInstance().setViewport(x, y, width, height);
}

void
RenderResources::setScissorRect(i32 x, i32 y, u32 width, u32 height)
{
  GraphicsDevice::getInstance().setScissor(x, y, width, height);
}

void
RenderResources::clearColor(std::optional<u32> attachment,
                            const glm::vec4& color)
{
  GraphicsDevice::getInstance().clearColor(attachment, color);
}

void
RenderResources::clearColor(std::optional<u32> attachment,
                            float r,
                            float g,
                            float b,
                            float a)
{
  GraphicsDevice::getInstance().clearColor(attachment, glm::vec4(r, g, b, a));
}

void
RenderResources::clearDepth(float depth)
{
  GraphicsDevice::getInstance().clearDepth(depth);
}

void
RenderResources::clearStencil(i32 value)
{
  GraphicsDevice::getInstance().clearStencil(value);
}

void
RenderResources::clearDepthStencil(float depth, i32 stencil)
{
  GraphicsDevice::getInstance().clearDepthStencil(depth, stencil);
}

void
RenderResources::setDepthTest(bool enable)
{
  GraphicsDevice::getInstance().setDepthTest(enable);
}

void
RenderResources::setDepthFunc(CompareOp op)
{
  GraphicsDevice::getInstance().setDepthFunc(op);
}

void
RenderResources::setDepthWrite(bool enable)
{
  GraphicsDevice::getInstance().setDepthWrite(enable);
}

void
RenderResources::setCullMode(CullMode mode)
{
  GraphicsDevice::getInstance().setCullMode(mode);
}

void
RenderResources::setFrontFace(FrontFace face)
{
  GraphicsDevice::getInstance().setFrontFace(face);
}

void
RenderResources::setBlendEnabled(bool enable)
{
  GraphicsDevice::getInstance().setBlendEnabled(enable);
}

void
RenderResources::setBlendFunc(BlendFactor srcColor,
                              BlendFactor dstColor,
                              BlendFactor srcAlpha,
                              BlendFactor dstAlpha)
{
  GraphicsDevice::getInstance().setBlendFunc(
    srcColor, dstColor, srcAlpha, dstAlpha);
}

void
RenderResources::setBlendFunc(BlendFactor src, BlendFactor dst)
{
  GraphicsDevice::getInstance().setBlendFunc(src, dst, src, dst);
}

void
RenderResources::setBlendEquation(BlendOp colorOp, BlendOp alphaOp)
{
  GraphicsDevice::getInstance().setBlendEquation(colorOp, alphaOp);
}

void
RenderResources::setScissorTest(bool enable)
{
  GraphicsDevice::getInstance().setScissorTest(enable);
}

void
RenderResources::setSeamlessCubemap(bool enable)
{
  GraphicsDevice::getInstance().setSeamlessCubemap(enable);
}

void
RenderResources::setLineWidth(float width)
{
  GraphicsDevice::getInstance().setLineWidth(width);
}

void
RenderResources::setLineSmoothing(bool enable)
{
  GraphicsDevice::getInstance().setLineSmoothing(enable);
}

void
RenderResources::setColorMask(bool red, bool green, bool blue, bool alpha)
{
  GraphicsDevice::getInstance().setColorMask(red, green, blue, alpha);
}

void
RenderResources::setBlendColor(float red, float green, float blue, float alpha)
{
  GraphicsDevice::getInstance().setBlendColor(red, green, blue, alpha);
}

void
RenderResources::setReadBuffer(std::optional<u32> attachment)
{
  GraphicsDevice::getInstance().setReadBuffer(attachment);
}

void
RenderResources::bindShaderProgram(const std::string& name)
{
  ShaderId shader = getShaderProgram(name);
  if (shader.isValid()) {
    GraphicsDevice::getInstance().bindShaderProgram(shader);
  }
}

i32
RenderResources::getUniformLocation(const std::string& shaderName,
                                    const char* uniformName) const
{
  ShaderId shader = getShaderProgram(shaderName);
  if (!shader.isValid()) {
    return -1;
  }
  return GraphicsDevice::getInstance().getUniformLocation(shader, uniformName);
}

i32
RenderResources::getAttribLocation(const std::string& shaderName,
                                   const char* attribName) const
{
  ShaderId shader = getShaderProgram(shaderName);
  if (!shader.isValid()) {
    return -1;
  }
  return GraphicsDevice::getInstance().getAttribLocation(shader, attribName);
}

void
RenderResources::bindTexture(u32 unit, const std::string& name)
{
  // First check regular textures managed by GraphicsDevice
  TextureId texture = getTexture(name);
  if (texture.isValid()) {
    GraphicsDevice::getInstance().bindTexture(unit, texture);
    return;
  }

  // Then check data textures (managed by GraphicsDevice with mutable storage)
  auto dataIt = m_dataTextures.find(name);
  if (dataIt != m_dataTextures.end()) {
    GraphicsDevice::getInstance().bindTexture(unit, dataIt->second);
    return;
  }

  // Texture not found - bind nothing (or could log warning)
  GraphicsDevice::getInstance().bindTexture(unit, TextureId{});
}

void
RenderResources::bindTexture(u32 unit, TextureId texture)
{
  GraphicsDevice::getInstance().bindTexture(unit, texture);
}

void
RenderResources::bindSampler(u32 unit, const std::string& name)
{
  SamplerId sampler = getSampler(name);
  GraphicsDevice::getInstance().bindSampler(unit, sampler);
}

void
RenderResources::bindSampler(u32 unit, SamplerId sampler)
{
  GraphicsDevice::getInstance().bindSampler(unit, sampler);
}

void
RenderResources::blitFramebuffer(const std::string& srcName,
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
                                 bool linearFilter)
{
  auto& device = GraphicsDevice::getInstance();

  FramebufferId srcFbo =
    srcName.empty() ? device.getDefaultFramebuffer() : getFramebuffer(srcName);
  FramebufferId dstFbo =
    dstName.empty() ? device.getDefaultFramebuffer() : getFramebuffer(dstName);

  device.blitFramebuffer(srcFbo,
                         dstFbo,
                         srcX0,
                         srcY0,
                         srcX1,
                         srcY1,
                         dstX0,
                         dstY0,
                         dstX1,
                         dstY1,
                         colorBit,
                         depthBit,
                         stencilBit,
                         linearFilter);
}

RenderResources::GeometryCreateResult
RenderResources::createGeometry(const void* vertexData,
                                u64 vertexDataSize,
                                const void* indexData,
                                u64 indexDataSize,
                                std::span<const VertexBinding> bindings,
                                std::span<const VertexAttribute> attributes,
                                PrimitiveTopology topology,
                                u32 vertexCount,
                                const char* debugName)
{
  auto& device = GraphicsDevice::getInstance();
  GeometryCreateResult result{};

  // Create vertex buffer
  BufferCreateInfo vboInfo{};
  vboInfo.size = vertexDataSize;
  vboInfo.usage = BufferUsage::Vertex;
  vboInfo.initialData = vertexData;
  result.vbo = device.createBuffer(vboInfo);

  // Create index buffer if index data is provided
  if (indexData != nullptr && indexDataSize > 0) {
    BufferCreateInfo eboInfo{};
    eboInfo.size = indexDataSize;
    eboInfo.usage = BufferUsage::Index;
    eboInfo.initialData = indexData;
    result.ebo = device.createBuffer(eboInfo);
  }

  // Create vertex array object
  VertexArrayCreateInfo vaoInfo{};
  vaoInfo.vertexBindings = bindings;
  vaoInfo.vertexAttributes = attributes;
  vaoInfo.topology = topology;
  vaoInfo.vertexCount = vertexCount;
  vaoInfo.debugName = debugName;
  result.vao = device.createVertexArray(vaoInfo);

  return result;
}

} // namespace gfx
