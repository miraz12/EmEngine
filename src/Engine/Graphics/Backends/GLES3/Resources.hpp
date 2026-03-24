#pragma once

#include "../../CommandBuffer.hpp"
#include "../../GraphicsTypes.hpp"
#include "../../Handle.hpp"
#include "../../Resources/Buffer.hpp"
#include "../../Resources/Framebuffer.hpp"
#include "../../Resources/Pipeline.hpp"
#include "../../Resources/Renderbuffer.hpp"
#include "../../Resources/Sampler.hpp"
#include "../../Resources/Shader.hpp"
#include "../../Resources/Texture.hpp"
#include "engine_pch.hpp"
#include <queue>
#include <unordered_map>

namespace gfx::gles3 {

/// GL buffer resource
struct Buffer
{
  GLuint glName{ 0 };
  u64 size{ 0 };
  BufferUsage usage{ BufferUsage::None };
};

/// GL texture resource
struct Texture
{
  GLuint glName{ 0 };
  GLenum glTarget{ GL_TEXTURE_2D };
  TextureCreateInfo info;
};

/// GL sampler resource
struct Sampler
{
  GLuint glName{ 0 };
  SamplerCreateInfo info;
};

/// GL shader program resource
struct ShaderProgram
{
  GLuint glName{ 0 };
  bool isLinkedProgram{ false };
  std::unordered_map<std::string, GLint> uniformCache;
  std::unordered_map<std::string, GLint> attribCache;
};

/// GL pipeline state (includes VAO for vertex format)
struct Pipeline
{
  ShaderId program;
  GLuint vao{ 0 }; // VAO caches vertex attribute setup
  std::vector<VertexBinding> vertexBindings;
  std::vector<VertexAttribute> vertexAttributes;
  PrimitiveTopology topology{ PrimitiveTopology::Triangles };
  RasterizerState rasterizer;
  DepthStencilState depthStencil;
  BlendState blend;
  u32 colorAttachmentCount{ 1 };
};

/// GL framebuffer resource
struct Framebuffer
{
  GLuint glName{ 0 };
  u32 width{ 0 };
  u32 height{ 0 };
  u32 colorAttachmentCount{ 0 };
  bool isDefault{ false };
};

/// GL renderbuffer resource
struct Renderbuffer
{
  GLuint glName{ 0 };
  u32 width{ 0 };
  u32 height{ 0 };
  PixelFormat format{ PixelFormat::Depth24Stencil8 };
};

/// GL vertex array object resource
struct VertexArray
{
  GLuint glName{ 0 };
  std::vector<VertexBinding> bindings;
  std::vector<VertexAttribute> attributes;
  PrimitiveTopology topology{ PrimitiveTopology::Triangles };
  u32 vertexCount{ 0 };
};

/// Generic resource pool with generation-based handle management
template<typename T, typename HandleType>
class ResourcePool
{
public:
  ResourcePool() = default;
  ~ResourcePool() = default;

  // Non-copyable
  ResourcePool(const ResourcePool&) = delete;
  ResourcePool& operator=(const ResourcePool&) = delete;

  /// Allocate a new resource and return its handle
  HandleType allocate(T&& resource)
  {
    u32 index;
    if (!m_freeList.empty()) {
      index = m_freeList.front();
      m_freeList.pop();
      m_resources[index] = std::move(resource);
    } else {
      index = static_cast<u32>(m_resources.size());
      m_resources.push_back(std::move(resource));
      m_generations.push_back(0);
    }
    // Index 0 is reserved for invalid handle, so shift by 1
    return HandleType::create(index + 1, m_generations[index]);
  }

  /// Release a resource by handle
  void release(HandleType handle)
  {
    if (!isValid(handle)) {
      return;
    }
    u32 index = handle.index() - 1;
    m_generations[index]++;
    assert(m_generations[index] != 0 && "Handle generation wrapped");
    m_freeList.push(index);
  }

  /// Get mutable resource by handle
  T* get(HandleType handle)
  {
    if (!isValid(handle)) {
      return nullptr;
    }
    return &m_resources[handle.index() - 1];
  }

  /// Get const resource by handle
  const T* get(HandleType handle) const
  {
    if (!isValid(handle)) {
      return nullptr;
    }
    return &m_resources[handle.index() - 1];
  }

  /// Check if handle is valid
  [[nodiscard]] bool isValid(HandleType handle) const
  {
    if (!handle.isValid()) {
      return false;
    }
    u32 index = handle.index() - 1;
    if (index >= m_resources.size()) {
      return false;
    }
    return handle.generation() == m_generations[index];
  }

  /// Get all resources (for cleanup)
  std::vector<T>& getAll() { return m_resources; }
  const std::vector<T>& getAll() const { return m_resources; }

private:
  std::vector<T> m_resources;
  std::vector<u8> m_generations;
  std::queue<u32> m_freeList;
};

} // namespace gfx::gles3
