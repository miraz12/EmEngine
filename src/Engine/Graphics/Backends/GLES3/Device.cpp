#include "Device.hpp"
#include <Graphics/RenderResources.hpp>
#include <cstring>
#include <iostream>

namespace gfx::gles3 {

Device::~Device()
{
  if (m_initialized) {
    shutdown();
  }
}

bool
Device::initialize()
{
  if (m_initialized) {
    return true;
  }

  // Create default framebuffer handle (GL name 0)
  Framebuffer defaultFb;
  defaultFb.glName = 0;
  defaultFb.isDefault = true;
  m_defaultFramebuffer = m_framebuffers.allocate(std::move(defaultFb));

  m_stateCache.reset();
  m_initialized = true;
  return true;
}

void
Device::shutdown()
{
  if (!m_initialized) {
    return;
  }

  // Delete all pending resources immediately
  for (auto& queue : m_pendingBufferDeletes) {
    for (GLuint name : queue) {
      glDeleteBuffers(1, &name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingTextureDeletes) {
    for (GLuint name : queue) {
      glDeleteTextures(1, &name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingSamplerDeletes) {
    for (GLuint name : queue) {
      glDeleteSamplers(1, &name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingShaderDeletes) {
    for (GLuint name : queue) {
      glDeleteShader(name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingProgramDeletes) {
    for (GLuint name : queue) {
      glDeleteProgram(name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingFramebufferDeletes) {
    for (GLuint name : queue) {
      glDeleteFramebuffers(1, &name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingVAODeletes) {
    for (GLuint name : queue) {
      glDeleteVertexArrays(1, &name);
    }
    queue.clear();
  }
  for (auto& queue : m_pendingRenderbufferDeletes) {
    for (GLuint name : queue) {
      glDeleteRenderbuffers(1, &name);
    }
    queue.clear();
  }

  // Delete remaining live resources
  for (auto& buffer : m_buffers.getAll()) {
    if (buffer.glName != 0) {
      glDeleteBuffers(1, &buffer.glName);
    }
  }
  for (auto& texture : m_textures.getAll()) {
    if (texture.glName != 0) {
      glDeleteTextures(1, &texture.glName);
    }
  }
  for (auto& sampler : m_samplers.getAll()) {
    if (sampler.glName != 0) {
      glDeleteSamplers(1, &sampler.glName);
    }
  }
  for (auto& shader : m_shaders.getAll()) {
    if (shader.glName != 0) {
      glDeleteProgram(shader.glName);
    }
  }
  for (auto& fb : m_framebuffers.getAll()) {
    if (fb.glName != 0 && !fb.isDefault) {
      glDeleteFramebuffers(1, &fb.glName);
    }
  }
  for (auto& pipe : m_pipelines.getAll()) {
    if (pipe.vao != 0) {
      glDeleteVertexArrays(1, &pipe.vao);
    }
  }
  for (auto& rbo : m_renderbuffers.getAll()) {
    if (rbo.glName != 0) {
      glDeleteRenderbuffers(1, &rbo.glName);
    }
  }

  m_initialized = false;
}

BufferId
Device::createBuffer(const BufferCreateInfo& info)
{
  Buffer buffer;
  buffer.size = info.size;
  buffer.usage = info.usage;

  glGenBuffers(1, &buffer.glName);

  GLenum target = GL_ARRAY_BUFFER;
  if (hasFlag(info.usage, BufferUsage::Index)) {
    target = GL_ELEMENT_ARRAY_BUFFER;
  } else if (hasFlag(info.usage, BufferUsage::Uniform)) {
    target = GL_UNIFORM_BUFFER;
  }

  glBindBuffer(target, buffer.glName);
  glBufferData(target,
               static_cast<GLsizeiptr>(info.size),
               info.initialData,
               toGLBufferUsage(info.usage));
  glBindBuffer(target, 0);

  return m_buffers.allocate(std::move(buffer));
}

void
Device::destroyBuffer(BufferId buffer)
{
  auto* res = m_buffers.get(buffer);
  if (res && res->glName != 0) {
    m_pendingBufferDeletes[m_frameIndex % kDeletionDelay].push_back(
      res->glName);
    res->glName = 0;
  }
  m_buffers.release(buffer);
}

void
Device::updateBuffer(BufferId buffer, u64 offset, const void* data, u64 size)
{
  auto* res = m_buffers.get(buffer);
  if (!res || res->glName == 0) {
    return;
  }

  GLenum target = GL_ARRAY_BUFFER;
  if (hasFlag(res->usage, BufferUsage::Index)) {
    target = GL_ELEMENT_ARRAY_BUFFER;
  } else if (hasFlag(res->usage, BufferUsage::Uniform)) {
    target = GL_UNIFORM_BUFFER;
  }

  glBindBuffer(target, res->glName);
  glBufferSubData(
    target, static_cast<GLintptr>(offset), static_cast<GLsizeiptr>(size), data);
  glBindBuffer(target, 0);
}

void*
Device::mapBuffer(BufferId buffer)
{
  auto* res = m_buffers.get(buffer);
  if (!res || res->glName == 0) {
    return nullptr;
  }

  GLenum target = GL_ARRAY_BUFFER;
  if (hasFlag(res->usage, BufferUsage::Index)) {
    target = GL_ELEMENT_ARRAY_BUFFER;
  } else if (hasFlag(res->usage, BufferUsage::Uniform)) {
    target = GL_UNIFORM_BUFFER;
  }

  glBindBuffer(target, res->glName);
  return glMapBufferRange(target,
                          0,
                          static_cast<GLsizeiptr>(res->size),
                          GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
}

void
Device::unmapBuffer(BufferId buffer)
{
  auto* res = m_buffers.get(buffer);
  if (!res || res->glName == 0) {
    return;
  }

  GLenum target = GL_ARRAY_BUFFER;
  if (hasFlag(res->usage, BufferUsage::Index)) {
    target = GL_ELEMENT_ARRAY_BUFFER;
  } else if (hasFlag(res->usage, BufferUsage::Uniform)) {
    target = GL_UNIFORM_BUFFER;
  }

  glBindBuffer(target, res->glName);
  glUnmapBuffer(target);
  glBindBuffer(target, 0);
}

TextureId
Device::createTexture(const TextureCreateInfo& info)
{
  Texture texture;
  texture.info = info;
  texture.glTarget = toGLTarget(info.type);

  glGenTextures(1, &texture.glName);
  glBindTexture(texture.glTarget, texture.glName);

  GLenum internalFormat = toGLInternalFormat(info.format);
  GLenum format = toGLFormat(info.format);
  GLenum type = toGLType(info.format);

  // Use glTexStorage for immutable texture allocation (allocates all mip
  // levels)
  GLsizei levels =
    static_cast<GLsizei>(info.mipLevels > 0 ? info.mipLevels : 1);

  if (info.storageMode == TextureStorageMode::Mutable) {
    // Mutable storage: use glTexImage2D so texture can be resized later
    switch (info.type) {
      case TextureType::Texture2D:
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     static_cast<GLint>(internalFormat),
                     static_cast<GLsizei>(info.width),
                     static_cast<GLsizei>(info.height),
                     0,
                     format,
                     type,
                     info.initialData);
        break;
      case TextureType::Texture2DArray:
      case TextureType::Texture3D:
      case TextureType::TextureCube:
        // Mutable storage only supported for 2D textures currently
        assert(false && "Mutable storage only supported for Texture2D");
        break;
    }
  } else {
    // Immutable storage: use glTexStorage (cannot resize)
    switch (info.type) {
      case TextureType::Texture2D:
        glTexStorage2D(GL_TEXTURE_2D,
                       levels,
                       internalFormat,
                       static_cast<GLsizei>(info.width),
                       static_cast<GLsizei>(info.height));
        if (info.initialData) {
          glTexSubImage2D(GL_TEXTURE_2D,
                          0,
                          0,
                          0,
                          static_cast<GLsizei>(info.width),
                          static_cast<GLsizei>(info.height),
                          format,
                          type,
                          info.initialData);
        }
        break;
      case TextureType::Texture2DArray:
        glTexStorage3D(GL_TEXTURE_2D_ARRAY,
                       levels,
                       internalFormat,
                       static_cast<GLsizei>(info.width),
                       static_cast<GLsizei>(info.height),
                       static_cast<GLsizei>(info.depthOrLayers));
        if (info.initialData) {
          glTexSubImage3D(GL_TEXTURE_2D_ARRAY,
                          0,
                          0,
                          0,
                          0,
                          static_cast<GLsizei>(info.width),
                          static_cast<GLsizei>(info.height),
                          static_cast<GLsizei>(info.depthOrLayers),
                          format,
                          type,
                          info.initialData);
        }
        break;
      case TextureType::TextureCube:
        glTexStorage2D(GL_TEXTURE_CUBE_MAP,
                       levels,
                       internalFormat,
                       static_cast<GLsizei>(info.width),
                       static_cast<GLsizei>(info.height));
        // Cube map initial data upload would need per-face data, skip for now
        break;
      case TextureType::Texture3D:
        glTexStorage3D(GL_TEXTURE_3D,
                       levels,
                       internalFormat,
                       static_cast<GLsizei>(info.width),
                       static_cast<GLsizei>(info.height),
                       static_cast<GLsizei>(info.depthOrLayers));
        if (info.initialData) {
          glTexSubImage3D(GL_TEXTURE_3D,
                          0,
                          0,
                          0,
                          0,
                          static_cast<GLsizei>(info.width),
                          static_cast<GLsizei>(info.height),
                          static_cast<GLsizei>(info.depthOrLayers),
                          format,
                          type,
                          info.initialData);
        }
        break;
    }
  }

  // Default filtering - use mipmap filtering if we have multiple levels
  // Mutable textures use NEAREST (data lookup textures)
  if (info.storageMode == TextureStorageMode::Mutable) {
    glTexParameteri(texture.glTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(texture.glTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  } else if (levels > 1) {
    glTexParameteri(
      texture.glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(texture.glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  } else {
    glTexParameteri(texture.glTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(texture.glTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  }
  glTexParameteri(texture.glTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(texture.glTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  // Cubemaps and 3D textures need WRAP_R for the third coordinate
  if (info.type == TextureType::TextureCube ||
      info.type == TextureType::Texture3D) {
    glTexParameteri(texture.glTarget, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  }

  if (info.format == PixelFormat::Depth16 ||
      info.format == PixelFormat::Depth24 ||
      info.format == PixelFormat::Depth32F) {
    glTexParameteri(
      texture.glTarget, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glTexParameteri(texture.glTarget, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
  }

  glBindTexture(texture.glTarget, 0);

  return m_textures.allocate(std::move(texture));
}

void
Device::destroyTexture(TextureId texture)
{
  auto* res = m_textures.get(texture);
  if (res && res->glName != 0) {
    m_pendingTextureDeletes[m_frameIndex % kDeletionDelay].push_back(
      res->glName);
    res->glName = 0;
  }
  m_textures.release(texture);
}

void
Device::updateTexture(TextureId texture,
                      u32 mipLevel,
                      u32 layer,
                      const void* data,
                      u64 /*dataSize*/)
{
  auto* res = m_textures.get(texture);
  if (!res || res->glName == 0) {
    return;
  }

  glBindTexture(res->glTarget, res->glName);

  GLenum format = toGLFormat(res->info.format);
  GLenum type = toGLType(res->info.format);
  u32 mipWidth = std::max(1u, res->info.width >> mipLevel);
  u32 mipHeight = std::max(1u, res->info.height >> mipLevel);

  switch (res->info.type) {
    case TextureType::Texture2D:
      glTexSubImage2D(GL_TEXTURE_2D,
                      static_cast<GLint>(mipLevel),
                      0,
                      0,
                      static_cast<GLsizei>(mipWidth),
                      static_cast<GLsizei>(mipHeight),
                      format,
                      type,
                      data);
      break;
    case TextureType::Texture2DArray:
    case TextureType::Texture3D:
      glTexSubImage3D(res->glTarget,
                      static_cast<GLint>(mipLevel),
                      0,
                      0,
                      static_cast<GLint>(layer),
                      static_cast<GLsizei>(mipWidth),
                      static_cast<GLsizei>(mipHeight),
                      1,
                      format,
                      type,
                      data);
      break;
    case TextureType::TextureCube:
      glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
                      static_cast<GLint>(mipLevel),
                      0,
                      0,
                      static_cast<GLsizei>(mipWidth),
                      static_cast<GLsizei>(mipHeight),
                      format,
                      type,
                      data);
      break;
  }

  glBindTexture(res->glTarget, 0);
}

void
Device::resizeTexture(TextureId texture,
                      u32 newWidth,
                      u32 newHeight,
                      const void* data)
{
  auto* res = m_textures.get(texture);
  if (!res || res->glName == 0) {
    return;
  }

  assert(res->info.storageMode == TextureStorageMode::Mutable &&
         "resizeTexture only works on mutable-storage textures");

  glBindTexture(res->glTarget, res->glName);

  GLenum internalFormat = toGLInternalFormat(res->info.format);
  GLenum format = toGLFormat(res->info.format);
  GLenum type = toGLType(res->info.format);

  glTexImage2D(GL_TEXTURE_2D,
               0,
               static_cast<GLint>(internalFormat),
               static_cast<GLsizei>(newWidth),
               static_cast<GLsizei>(newHeight),
               0,
               format,
               type,
               data);

  // Update stored dimensions
  res->info.width = newWidth;
  res->info.height = newHeight;

  glBindTexture(res->glTarget, 0);
}

void
Device::generateMipmaps(TextureId texture)
{
  auto* res = m_textures.get(texture);
  if (!res || res->glName == 0) {
    return;
  }

  glBindTexture(res->glTarget, res->glName);
  glGenerateMipmap(res->glTarget);
  glBindTexture(res->glTarget, 0);
}

SamplerId
Device::createSampler(const SamplerCreateInfo& info)
{
  Sampler sampler;
  sampler.info = info;

  glGenSamplers(1, &sampler.glName);

  glSamplerParameteri(
    sampler.glName, GL_TEXTURE_MIN_FILTER, toGLFilter(info.minFilter));
  glSamplerParameteri(
    sampler.glName, GL_TEXTURE_MAG_FILTER, toGLMagFilter(info.magFilter));
  glSamplerParameteri(sampler.glName, GL_TEXTURE_WRAP_S, toGLWrap(info.wrapU));
  glSamplerParameteri(sampler.glName, GL_TEXTURE_WRAP_T, toGLWrap(info.wrapV));
  glSamplerParameteri(sampler.glName, GL_TEXTURE_WRAP_R, toGLWrap(info.wrapW));

  if (info.compareEnable) {
    glSamplerParameteri(
      sampler.glName, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
    glSamplerParameteri(
      sampler.glName, GL_TEXTURE_COMPARE_FUNC, toGLCompareFunc(info.compareOp));
  }

  return m_samplers.allocate(std::move(sampler));
}

void
Device::destroySampler(SamplerId sampler)
{
  auto* res = m_samplers.get(sampler);
  if (res && res->glName != 0) {
    m_pendingSamplerDeletes[m_frameIndex % kDeletionDelay].push_back(
      res->glName);
    res->glName = 0;
  }
  m_samplers.release(sampler);
}

ShaderId
Device::createShader(const ShaderCreateInfo& info)
{
  GLenum glStage =
    (info.stage == ShaderStage::Vertex) ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER;

  GLuint shader = glCreateShader(glStage);
  const char* source = info.source;
  GLint length = static_cast<GLint>(info.sourceLength);
  glShaderSource(shader, 1, &source, &length);
  glCompileShader(shader);

  GLint success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
    std::cerr << "Shader compilation failed";
    if (info.debugName) {
      std::cerr << " (" << info.debugName << ")";
    }
    std::cerr << ": " << infoLog << std::endl;
    glDeleteShader(shader);
    return ShaderId{};
  }

  ShaderProgram prog;
  prog.glName = shader;
  prog.isLinkedProgram = false;

  return m_shaders.allocate(std::move(prog));
}

ShaderId
Device::createShaderProgram(const ShaderProgramCreateInfo& info)
{
  auto* vertShader = m_shaders.get(info.vertexShader);
  auto* fragShader = m_shaders.get(info.fragmentShader);

  if (!vertShader || !fragShader) {
    std::cerr << "Invalid shader handles for program creation" << std::endl;
    return ShaderId{};
  }

  GLuint program = glCreateProgram();
  glAttachShader(program, vertShader->glName);
  glAttachShader(program, fragShader->glName);
  glLinkProgram(program);

  GLint success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    glGetProgramInfoLog(program, sizeof(infoLog), nullptr, infoLog);
    std::cerr << "Shader program linking failed";
    if (info.debugName) {
      std::cerr << " (" << info.debugName << ")";
    }
    std::cerr << ": " << infoLog << std::endl;
    glDeleteProgram(program);
    return ShaderId{};
  }

  ShaderProgram prog;
  prog.glName = program;
  prog.isLinkedProgram = true;

  return m_shaders.allocate(std::move(prog));
}

void
Device::destroyShader(ShaderId shader)
{
  auto* res = m_shaders.get(shader);
  if (res && res->glName != 0) {
    // Use the correct deletion queue based on whether this is a linked program
    // or an individual shader object
    if (res->isLinkedProgram) {
      m_pendingProgramDeletes[m_frameIndex % kDeletionDelay].push_back(
        res->glName);
    } else {
      m_pendingShaderDeletes[m_frameIndex % kDeletionDelay].push_back(
        res->glName);
    }
    res->glName = 0;
  }
  m_shaders.release(shader);
}

PipelineId
Device::createPipeline(const PipelineCreateInfo& info)
{
  Pipeline pipeline;
  pipeline.program = info.shaderProgram;
  pipeline.vertexBindings.assign(info.vertexBindings.begin(),
                                 info.vertexBindings.end());
  pipeline.vertexAttributes.assign(info.vertexAttributes.begin(),
                                   info.vertexAttributes.end());
  pipeline.topology = info.topology;
  pipeline.rasterizer = info.rasterizer;
  pipeline.depthStencil = info.depthStencil;
  pipeline.blend = info.blend;
  pipeline.colorAttachmentCount = info.colorAttachmentCount;

  // Note: In GLES3/WebGL2, we must create the VAO but actual vertex buffer
  // binding happens at draw time since we don't know the buffers yet.
  // The VAO stores the attribute format/pointer configuration.
  glGenVertexArrays(1, &pipeline.vao);
  glBindVertexArray(pipeline.vao);

  // Enable and configure all vertex attributes
  // The actual buffer bindings will be set when binding vertex buffers
  for (const auto& attrib : info.vertexAttributes) {
    glEnableVertexAttribArray(attrib.location);
  }

  glBindVertexArray(0);

  return m_pipelines.allocate(std::move(pipeline));
}

void
Device::destroyPipeline(PipelineId pipeline)
{
  auto* res = m_pipelines.get(pipeline);
  if (res && res->vao != 0) {
    m_pendingVAODeletes[m_frameIndex % kDeletionDelay].push_back(res->vao);
    res->vao = 0;
  }
  m_pipelines.release(pipeline);
}

FramebufferId
Device::createFramebuffer(const FramebufferCreateInfo& info)
{
  Framebuffer fb;
  fb.width = info.width;
  fb.height = info.height;
  fb.colorAttachmentCount = static_cast<u32>(info.colorAttachments.size());

  glGenFramebuffers(1, &fb.glName);
  glBindFramebuffer(GL_FRAMEBUFFER, fb.glName);

  std::vector<GLenum> drawBuffers;
  for (size_t i = 0; i < info.colorAttachments.size(); ++i) {
    const auto& att = info.colorAttachments[i];
    auto* tex = m_textures.get(att.texture);
    if (tex && tex->glName != 0) {
      if (tex->info.type == TextureType::Texture2DArray) {
        glFramebufferTextureLayer(GL_FRAMEBUFFER,
                                  GL_COLOR_ATTACHMENT0 + i,
                                  tex->glName,
                                  att.mipLevel,
                                  att.layer);
      } else if (tex->info.type == TextureType::TextureCube) {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + i,
                               GL_TEXTURE_CUBE_MAP_POSITIVE_X + att.layer,
                               tex->glName,
                               att.mipLevel);
      } else {
        glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0 + i,
                               tex->glTarget,
                               tex->glName,
                               att.mipLevel);
      }
      drawBuffers.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
  }

  if (!drawBuffers.empty()) {
    glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
  }

  if (info.depthStencilAttachment.texture.isValid()) {
    auto* depthTex = m_textures.get(info.depthStencilAttachment.texture);
    if (depthTex && depthTex->glName != 0) {
      GLenum attachment = GL_DEPTH_ATTACHMENT;
      if (depthTex->info.format == PixelFormat::Depth24Stencil8) {
        attachment = GL_DEPTH_STENCIL_ATTACHMENT;
      }
      glFramebufferTexture2D(GL_FRAMEBUFFER,
                             attachment,
                             depthTex->glTarget,
                             depthTex->glName,
                             info.depthStencilAttachment.mipLevel);
    }
  }

  // Only check completeness if we attached something
  // Bare FBOs (no attachments) are intentionally incomplete at creation
  bool hasAttachments = !info.colorAttachments.empty() ||
                        info.depthStencilAttachment.texture.isValid();
  if (hasAttachments) {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      std::cerr << "Framebuffer incomplete";
      if (info.debugName) {
        std::cerr << " (" << info.debugName << ")";
      }
      std::cerr << ": 0x" << std::hex << status << std::dec << std::endl;
    }
  }

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  m_stateCache.boundFBO = 0;

  return m_framebuffers.allocate(std::move(fb));
}

void
Device::destroyFramebuffer(FramebufferId framebuffer)
{
  auto* res = m_framebuffers.get(framebuffer);
  if (res && res->glName != 0 && !res->isDefault) {
    m_pendingFramebufferDeletes[m_frameIndex % kDeletionDelay].push_back(
      res->glName);
    res->glName = 0;
  }
  m_framebuffers.release(framebuffer);
}

void
Device::setFramebufferAttachment(FramebufferId fbo,
                                 u32 attachmentIndex,
                                 TextureId texture,
                                 u32 mipLevel,
                                 u32 layer)
{
  auto* framebuffer = m_framebuffers.get(fbo);
  auto* tex = m_textures.get(texture);
  if (!framebuffer || !tex) {
    return;
  }

  if (m_stateCache.boundFBO != framebuffer->glName) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->glName);
    m_stateCache.boundFBO = framebuffer->glName;
  }

  GLenum attachment = GL_COLOR_ATTACHMENT0 + attachmentIndex;

  if (tex->info.type == TextureType::TextureCube) {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           attachment,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
                           tex->glName,
                           mipLevel);
  } else if (tex->info.type == TextureType::Texture2DArray) {
    glFramebufferTextureLayer(
      GL_FRAMEBUFFER, attachment, tex->glName, mipLevel, layer);
  } else {
    glFramebufferTexture2D(
      GL_FRAMEBUFFER, attachment, tex->glTarget, tex->glName, mipLevel);
  }
}

void
Device::setFramebufferDepthAttachment(FramebufferId fbo,
                                      TextureId texture,
                                      u32 mipLevel,
                                      u32 layer)
{
  auto* framebuffer = m_framebuffers.get(fbo);
  auto* tex = m_textures.get(texture);
  if (!framebuffer || !tex) {
    return;
  }

  if (m_stateCache.boundFBO != framebuffer->glName) {
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->glName);
    m_stateCache.boundFBO = framebuffer->glName;
  }

  if (tex->info.type == TextureType::Texture2DArray) {
    glFramebufferTextureLayer(
      GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, tex->glName, mipLevel, layer);
  } else if (tex->info.type == TextureType::TextureCube) {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           GL_TEXTURE_CUBE_MAP_POSITIVE_X + layer,
                           tex->glName,
                           mipLevel);
  } else {
    glFramebufferTexture2D(GL_FRAMEBUFFER,
                           GL_DEPTH_ATTACHMENT,
                           tex->glTarget,
                           tex->glName,
                           mipLevel);
  }
}

void
Device::setFramebufferRenderbuffer(FramebufferId fbo,
                                   RenderbufferAttachment attachmentType,
                                   RenderbufferId rbo)
{
  auto* framebuffer = m_framebuffers.get(fbo);
  auto* renderbuffer = m_renderbuffers.get(rbo);
  if (!framebuffer || !renderbuffer) {
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->glName);

  GLenum attachment;
  switch (attachmentType) {
    case RenderbufferAttachment::Depth:
      attachment = GL_DEPTH_ATTACHMENT;
      break;
    case RenderbufferAttachment::Stencil:
      attachment = GL_STENCIL_ATTACHMENT;
      break;
    case RenderbufferAttachment::DepthStencil:
      attachment = GL_DEPTH_STENCIL_ATTACHMENT;
      break;
  }

  glFramebufferRenderbuffer(
    GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbuffer->glName);
}

void
Device::setDrawBuffers(FramebufferId fbo, std::span<const u32> attachments)
{
  auto* framebuffer = m_framebuffers.get(fbo);
  if (!framebuffer) {
    return;
  }

  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->glName);

  if (attachments.empty()) {
    // No color attachments (depth-only)
    GLenum none = GL_NONE;
    glDrawBuffers(1, &none);
  } else {
    std::vector<GLenum> glAttachments;
    glAttachments.reserve(attachments.size());
    for (u32 idx : attachments) {
      glAttachments.push_back(GL_COLOR_ATTACHMENT0 + idx);
    }
    glDrawBuffers(static_cast<GLsizei>(glAttachments.size()),
                  glAttachments.data());
  }
}

RenderbufferId
Device::createRenderbuffer(const RenderbufferCreateInfo& info)
{
  GLuint rbo;
  glGenRenderbuffers(1, &rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo);

  GLenum internalFormat = toGLInternalFormat(info.format);
  glRenderbufferStorage(
    GL_RENDERBUFFER, internalFormat, info.width, info.height);

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  if (info.debugName) {
    glObjectLabel(GL_RENDERBUFFER, rbo, -1, info.debugName);
  }
#endif

  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  Renderbuffer renderbuffer{};
  renderbuffer.glName = rbo;
  renderbuffer.width = info.width;
  renderbuffer.height = info.height;
  renderbuffer.format = info.format;

  return m_renderbuffers.allocate(std::move(renderbuffer));
}

void
Device::destroyRenderbuffer(RenderbufferId renderbuffer)
{
  auto* res = m_renderbuffers.get(renderbuffer);
  if (res && res->glName != 0) {
    m_pendingRenderbufferDeletes[m_frameIndex % kDeletionDelay].push_back(
      res->glName);
    res->glName = 0;
  }
  m_renderbuffers.release(renderbuffer);
}

void
Device::resizeRenderbuffer(RenderbufferId renderbuffer, u32 width, u32 height)
{
  auto* res = m_renderbuffers.get(renderbuffer);
  if (!res || res->glName == 0) {
    return;
  }

  glBindRenderbuffer(GL_RENDERBUFFER, res->glName);
  GLenum internalFormat = toGLInternalFormat(res->format);
  glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  res->width = width;
  res->height = height;
}

VertexArrayId
Device::createVertexArray(const VertexArrayCreateInfo& info)
{
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  // OpenGL ES 3.0 / Core Profile requires a buffer to be bound when calling
  // glVertexAttribPointer, so we defer that to drawVertexArray when the
  // actual VBO is bound.
  for (const auto& attrib : info.vertexAttributes) {
    glEnableVertexAttribArray(attrib.location);
  }

#if !defined(EMSCRIPTEN) && !defined(NDEBUG)
  if (info.debugName) {
    glObjectLabel(GL_VERTEX_ARRAY, vao, -1, info.debugName);
  }
#endif

  glBindVertexArray(0);

  VertexArray vertexArray{};
  vertexArray.glName = vao;
  vertexArray.bindings = { info.vertexBindings.begin(),
                           info.vertexBindings.end() };
  vertexArray.attributes = { info.vertexAttributes.begin(),
                             info.vertexAttributes.end() };
  vertexArray.topology = info.topology;
  vertexArray.vertexCount = info.vertexCount;

  return m_vertexArrays.allocate(std::move(vertexArray));
}

void
Device::destroyVertexArray(VertexArrayId vao)
{
  auto* res = m_vertexArrays.get(vao);
  if (res && res->glName != 0) {
    m_pendingVAODeletes[m_frameIndex % kDeletionDelay].push_back(res->glName);
    res->glName = 0;
  }
  m_vertexArrays.release(vao);
}

void
Device::bindVertexArray(VertexArrayId vao)
{
  auto* res = m_vertexArrays.get(vao);
  if (!res) {
    return;
  }

  if (m_stateCache.boundVAO != res->glName) {
    glBindVertexArray(res->glName);
    m_stateCache.boundVAO = res->glName;
  }
}

void
Device::drawVertexArray(VertexArrayId vao, BufferId vertexBuffer)
{
  auto* vaoRes = m_vertexArrays.get(vao);
  auto* bufRes = m_buffers.get(vertexBuffer);
  if (!vaoRes || !bufRes) {
    return;
  }

  if (m_stateCache.boundVAO != vaoRes->glName) {
    glBindVertexArray(vaoRes->glName);
    m_stateCache.boundVAO = vaoRes->glName;
  }

  glBindBuffer(GL_ARRAY_BUFFER, bufRes->glName);

  for (const auto& attrib : vaoRes->attributes) {
    u32 stride = 0;
    for (const auto& binding : vaoRes->bindings) {
      if (binding.binding == attrib.binding) {
        stride = binding.stride;
        break;
      }
    }

    auto info = toGLVertexAttrib(attrib.format);
    if (info.isInteger) {
      glVertexAttribIPointer(
        attrib.location,
        info.size,
        info.type,
        stride,
        reinterpret_cast<void*>(static_cast<uintptr_t>(attrib.offset)));
    } else {
      glVertexAttribPointer(
        attrib.location,
        info.size,
        info.type,
        info.normalized,
        stride,
        reinterpret_cast<void*>(static_cast<uintptr_t>(attrib.offset)));
    }
  }

  GLenum mode = toGLPrimitive(vaoRes->topology);
  glDrawArrays(mode, 0, vaoRes->vertexCount);

  // Unbind VAO to avoid interfering with legacy code that binds VAOs directly
  // (bypassing the state cache). This ensures Primitive::draw() etc. work
  // correctly.
  glBindVertexArray(0);
  m_stateCache.boundVAO = 0;
}

void
Device::drawVertexArrayDynamic(VertexArrayId vao,
                               BufferId vertexBuffer,
                               PrimitiveTopology topology,
                               u32 vertexCount)
{
  auto* vaoRes = m_vertexArrays.get(vao);
  auto* bufRes = m_buffers.get(vertexBuffer);
  if (!vaoRes || !bufRes || vertexCount == 0) {
    return;
  }

  if (m_stateCache.boundVAO != vaoRes->glName) {
    glBindVertexArray(vaoRes->glName);
    m_stateCache.boundVAO = vaoRes->glName;
  }

  glBindBuffer(GL_ARRAY_BUFFER, bufRes->glName);

  for (const auto& attrib : vaoRes->attributes) {
    u32 stride = 0;
    for (const auto& binding : vaoRes->bindings) {
      if (binding.binding == attrib.binding) {
        stride = binding.stride;
        break;
      }
    }

    auto info = toGLVertexAttrib(attrib.format);
    if (info.isInteger) {
      glVertexAttribIPointer(
        attrib.location,
        info.size,
        info.type,
        stride,
        reinterpret_cast<void*>(static_cast<uintptr_t>(attrib.offset)));
    } else {
      glVertexAttribPointer(
        attrib.location,
        info.size,
        info.type,
        info.normalized,
        stride,
        reinterpret_cast<void*>(static_cast<uintptr_t>(attrib.offset)));
    }
    glEnableVertexAttribArray(attrib.location);
  }

  GLenum mode = toGLPrimitive(topology);
  glDrawArrays(mode, 0, static_cast<GLsizei>(vertexCount));

  glBindVertexArray(0);
  m_stateCache.boundVAO = 0;
}

void
Device::drawIndexedVertexArray(VertexArrayId vao,
                               BufferId vertexBuffer,
                               BufferId indexBuffer,
                               u32 indexCount,
                               IndexType indexType,
                               u32 offset)
{
  auto* vaoRes = m_vertexArrays.get(vao);
  auto* vbufRes = m_buffers.get(vertexBuffer);
  auto* ibufRes = m_buffers.get(indexBuffer);
  if (!vaoRes || !vbufRes || !ibufRes) {
    return;
  }

  if (m_stateCache.boundVAO != vaoRes->glName) {
    glBindVertexArray(vaoRes->glName);
    m_stateCache.boundVAO = vaoRes->glName;
  }

  glBindBuffer(GL_ARRAY_BUFFER, vbufRes->glName);

  for (const auto& attrib : vaoRes->attributes) {
    u32 stride = 0;
    for (const auto& binding : vaoRes->bindings) {
      if (binding.binding == attrib.binding) {
        stride = binding.stride;
        break;
      }
    }

    auto info = toGLVertexAttrib(attrib.format);
    if (info.isInteger) {
      glVertexAttribIPointer(
        attrib.location,
        info.size,
        info.type,
        stride,
        reinterpret_cast<void*>(static_cast<uintptr_t>(attrib.offset)));
    } else {
      glVertexAttribPointer(
        attrib.location,
        info.size,
        info.type,
        info.normalized,
        stride,
        reinterpret_cast<void*>(static_cast<uintptr_t>(attrib.offset)));
    }
    glEnableVertexAttribArray(attrib.location);
  }

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibufRes->glName);

  GLenum mode = toGLPrimitive(vaoRes->topology);
  GLenum glIndexType = toGLIndexType(indexType);
  size_t indexSize = (indexType == IndexType::U32) ? 4 : 2;
  void* indexOffset =
    reinterpret_cast<void*>(static_cast<uintptr_t>(offset * indexSize));

  glDrawElements(
    mode, static_cast<GLsizei>(indexCount), glIndexType, indexOffset);

  glBindVertexArray(0);
  m_stateCache.boundVAO = 0;
}

bool
Device::isValid(VertexArrayId handle) const
{
  return m_vertexArrays.isValid(handle);
}

CommandBufferId
Device::createCommandBuffer()
{
  return m_commandBuffers.allocate(CommandBuffer{});
}

void
Device::destroyCommandBuffer(CommandBufferId cmdBuffer)
{
  m_commandBuffers.release(cmdBuffer);
}

CommandBuffer*
Device::getCommandBuffer(CommandBufferId cmdId)
{
  return m_commandBuffers.get(cmdId);
}

void
Device::beginFrame()
{
  processDeletionQueues();
}

void
Device::endFrame()
{
  m_frameIndex++;
}

void
Device::processDeletionQueues()
{
  u32 queueIndex = m_frameIndex % kDeletionDelay;

  for (GLuint name : m_pendingBufferDeletes[queueIndex]) {
    glDeleteBuffers(1, &name);
  }
  m_pendingBufferDeletes[queueIndex].clear();

  for (GLuint name : m_pendingTextureDeletes[queueIndex]) {
    glDeleteTextures(1, &name);
  }
  m_pendingTextureDeletes[queueIndex].clear();

  for (GLuint name : m_pendingSamplerDeletes[queueIndex]) {
    glDeleteSamplers(1, &name);
  }
  m_pendingSamplerDeletes[queueIndex].clear();

  // Delete individual shader objects (vertex/fragment shaders)
  for (GLuint name : m_pendingShaderDeletes[queueIndex]) {
    glDeleteShader(name);
  }
  m_pendingShaderDeletes[queueIndex].clear();

  // Delete linked shader programs
  for (GLuint name : m_pendingProgramDeletes[queueIndex]) {
    glDeleteProgram(name);
  }
  m_pendingProgramDeletes[queueIndex].clear();

  for (GLuint name : m_pendingFramebufferDeletes[queueIndex]) {
    glDeleteFramebuffers(1, &name);
  }
  m_pendingFramebufferDeletes[queueIndex].clear();

  for (GLuint name : m_pendingRenderbufferDeletes[queueIndex]) {
    glDeleteRenderbuffers(1, &name);
  }
  m_pendingRenderbufferDeletes[queueIndex].clear();

  for (GLuint name : m_pendingVAODeletes[queueIndex]) {
    glDeleteVertexArrays(1, &name);
  }
  m_pendingVAODeletes[queueIndex].clear();
}

bool
Device::isValid(BufferId handle) const
{
  return m_buffers.isValid(handle);
}

bool
Device::isValid(TextureId handle) const
{
  return m_textures.isValid(handle);
}

bool
Device::isValid(SamplerId handle) const
{
  return m_samplers.isValid(handle);
}

bool
Device::isValid(ShaderId handle) const
{
  return m_shaders.isValid(handle);
}

bool
Device::isValid(PipelineId handle) const
{
  return m_pipelines.isValid(handle);
}

bool
Device::isValid(FramebufferId handle) const
{
  return m_framebuffers.isValid(handle);
}

bool
Device::isFramebufferComplete(FramebufferId fbo) const
{
  // Default framebuffer is always complete
  if (fbo == m_defaultFramebuffer) {
    return true;
  }

  if (!m_framebuffers.isValid(fbo)) {
    return false;
  }

  const auto* res = m_framebuffers.get(fbo);
  if (!res) {
    return false;
  }

  GLint prevFbo;
  glGetIntegerv(GL_FRAMEBUFFER_BINDING, &prevFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, res->glName);
  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(prevFbo));

  return status == GL_FRAMEBUFFER_COMPLETE;
}

bool
Device::isValid(RenderbufferId handle) const
{
  return m_renderbuffers.isValid(handle);
}

GLuint
Device::getNativeHandle(BufferId buffer) const
{
  auto* res = m_buffers.get(buffer);
  return res ? res->glName : 0;
}

GLuint
Device::getNativeHandle(TextureId texture) const
{
  auto* res = m_textures.get(texture);
  return res ? res->glName : 0;
}

GLuint
Device::getNativeHandle(ShaderId program) const
{
  auto* res = m_shaders.get(program);
  return res ? res->glName : 0;
}

GLuint
Device::getNativeHandle(FramebufferId framebuffer) const
{
  auto* res = m_framebuffers.get(framebuffer);
  return res ? res->glName : 0;
}

GLuint
Device::getNativeHandle(RenderbufferId renderbuffer) const
{
  auto* res = m_renderbuffers.get(renderbuffer);
  return res ? res->glName : 0;
}

GLint
Device::getUniformLocation(ShaderId program, const char* name) const
{
  auto* res = m_shaders.get(program);
  if (!res || !res->isLinkedProgram) {
    return -1;
  }

  auto it = res->uniformCache.find(name);
  if (it != res->uniformCache.end()) {
    return it->second;
  }

  GLint loc = glGetUniformLocation(res->glName, name);
  // const_cast for cache update - cache is a mutable optimization
  const_cast<ShaderProgram*>(res)->uniformCache[name] = loc;
  return loc;
}

GLint
Device::getAttribLocation(ShaderId program, const char* name) const
{
  auto* res = m_shaders.get(program);
  if (!res || !res->isLinkedProgram) {
    return -1;
  }

  auto it = res->attribCache.find(name);
  if (it != res->attribCache.end()) {
    return it->second;
  }

  GLint loc = glGetAttribLocation(res->glName, name);
  const_cast<ShaderProgram*>(res)->attribCache[name] = loc;
  return loc;
}

void
Device::setUniformInt(i32 location, i32 value)
{
  if (location >= 0) {
    glUniform1i(location, value);
  }
}

void
Device::setUniformIntArray(i32 location, std::span<const i32> values)
{
  if (location >= 0 && !values.empty()) {
    glUniform1iv(location, static_cast<GLsizei>(values.size()), values.data());
  }
}

void
Device::setUniformFloat(i32 location, float value)
{
  if (location >= 0) {
    glUniform1f(location, value);
  }
}

void
Device::setUniformVec2(i32 location, const glm::vec2& value)
{
  if (location >= 0) {
    glUniform2fv(location, 1, glm::value_ptr(value));
  }
}

void
Device::setUniformVec3(i32 location, const glm::vec3& value)
{
  if (location >= 0) {
    glUniform3fv(location, 1, glm::value_ptr(value));
  }
}

void
Device::setUniformVec4(i32 location, const glm::vec4& value)
{
  if (location >= 0) {
    glUniform4fv(location, 1, glm::value_ptr(value));
  }
}

void
Device::setUniformMat3(i32 location, const glm::mat3& value)
{
  if (location >= 0) {
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
}

void
Device::setUniformMat4(i32 location, const glm::mat4& value)
{
  if (location >= 0) {
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
  }
}

FramebufferId
Device::getDefaultFramebuffer() const
{
  return m_defaultFramebuffer;
}

void
Device::executeCommandBuffer(const CommandBuffer& cmdBuffer)
{
  const auto& stream = cmdBuffer.getCommandStream();
  const auto& debugNames = cmdBuffer.getDebugNames();

  size_t offset = 0;
  auto read = [&]<typename T>(T& value) {
    assert(offset + sizeof(T) <= stream.size() &&
           "Command buffer read out of bounds");
    std::memcpy(&value, stream.data() + offset, sizeof(T));
    offset += sizeof(T);
  };

  while (offset < stream.size()) {
    CommandType cmd;
    read(cmd);

    switch (cmd) {
      case CommandType::BeginRenderPass: {
        RenderPassBeginInfo info;
        read(info);
        executeBeginRenderPass(info);
        break;
      }
      case CommandType::EndRenderPass:
        executeEndRenderPass();
        break;
      case CommandType::BindPipeline: {
        PipelineId pipeline;
        read(pipeline);
        executeBindPipeline(pipeline);
        break;
      }
      case CommandType::SetViewport: {
        Viewport viewport;
        read(viewport);
        executeSetViewport(viewport);
        break;
      }
      case CommandType::SetScissor: {
        ScissorRect scissor;
        read(scissor);
        executeSetScissor(scissor);
        break;
      }
      case CommandType::BindVertexBuffer: {
        u32 binding;
        BufferId buffer;
        u64 bufOffset;
        read(binding);
        read(buffer);
        read(bufOffset);
        executeBindVertexBuffer(binding, buffer, bufOffset);
        break;
      }
      case CommandType::BindIndexBuffer: {
        BufferId buffer;
        u64 bufOffset;
        IndexType indexType;
        read(buffer);
        read(bufOffset);
        read(indexType);
        executeBindIndexBuffer(buffer, bufOffset, indexType);
        break;
      }
      case CommandType::BindUniformBuffer: {
        u32 binding;
        BufferId buffer;
        u64 bufOffset;
        u64 size;
        read(binding);
        read(buffer);
        read(bufOffset);
        read(size);
        auto* res = m_buffers.get(buffer);
        if (res) {
          glBindBufferRange(GL_UNIFORM_BUFFER,
                            binding,
                            res->glName,
                            static_cast<GLintptr>(bufOffset),
                            static_cast<GLsizeiptr>(size));
        }
        break;
      }
      case CommandType::BindTexture: {
        u32 slot;
        TextureId texture;
        SamplerId sampler;
        read(slot);
        read(texture);
        read(sampler);
        executeBindTexture(slot, texture, sampler);
        break;
      }
      case CommandType::SetFramebufferAttachment: {
        FramebufferId fbo;
        u32 attachmentIndex;
        TextureId texture;
        u32 mipLevel;
        u32 layer;
        read(fbo);
        read(attachmentIndex);
        read(texture);
        read(mipLevel);
        read(layer);
        setFramebufferAttachment(
          fbo, attachmentIndex, texture, mipLevel, layer);
        break;
      }
      case CommandType::SetFramebufferDepthAttachment: {
        FramebufferId fbo;
        TextureId texture;
        u32 mipLevel;
        u32 layer;
        read(fbo);
        read(texture);
        read(mipLevel);
        read(layer);
        setFramebufferDepthAttachment(fbo, texture, mipLevel, layer);
        break;
      }
      case CommandType::ClearDepth: {
        float depth;
        read(depth);
        clearDepth(depth);
        break;
      }
      case CommandType::BindVertexArray: {
        VertexArrayId vao;
        read(vao);
        auto* vaoRes = m_vertexArrays.get(vao);
        if (vaoRes) {
          glBindVertexArray(vaoRes->glName);
          m_stateCache.boundVAO = vaoRes->glName;
          m_stateCache.explicitVAO =
            vao; // Track explicit VAO for vertex buffer binding
        }
        break;
      }
      case CommandType::BindTextureByName: {
        u32 slot;
        u32 len;
        SamplerId sampler;
        read(slot);
        read(len);
        assert(offset + len <= stream.size() &&
               "Command buffer string read out of bounds");
        std::string textureName(
          reinterpret_cast<const char*>(stream.data() + offset), len);
        offset += len;
        read(sampler);
        // Use RenderResources::bindTexture which handles both regular textures
        // (m_textures) and data textures (m_dataTextures like jointMats)
        auto& resources = RenderResources::getInstance();
        resources.bindTexture(slot, textureName);
        auto* samp = m_samplers.get(sampler);
        if (samp) {
          glBindSampler(slot, samp->glName);
          m_stateCache.boundSamplers[slot] = samp->glName;
        }
        break;
      }
      case CommandType::SetCullMode: {
        CullMode mode;
        read(mode);
        if (mode != CullMode::None) {
          glEnable(GL_CULL_FACE);
          glCullFace(mode == CullMode::Front ? GL_FRONT : GL_BACK);
        } else {
          glDisable(GL_CULL_FACE);
        }
        break;
      }
      case CommandType::SetBlendEnabled: {
        bool enabled;
        read(enabled);
        if (enabled) {
          glEnable(GL_BLEND);
        } else {
          glDisable(GL_BLEND);
        }
        break;
      }
      case CommandType::SetBlendFunc: {
        BlendFactor srcColor, dstColor, srcAlpha, dstAlpha;
        read(srcColor);
        read(dstColor);
        read(srcAlpha);
        read(dstAlpha);
        glBlendFuncSeparate(toGLBlendFactor(srcColor),
                            toGLBlendFactor(dstColor),
                            toGLBlendFactor(srcAlpha),
                            toGLBlendFactor(dstAlpha));
        break;
      }
      case CommandType::FlushMaterialUBO: {
        auto& resources = RenderResources::getInstance();
        resources.flushMaterialUBO();
        break;
      }
      case CommandType::UpdateMaterialUBO: {
        MaterialUBO data;
        read(data);
        auto& resources = RenderResources::getInstance();
        resources.getMaterialUBO() = data;
        resources.flushMaterialUBO();
        break;
      }
      case CommandType::SetUniformMat4: {
        i32 location;
        glm::mat4 value;
        read(location);
        read(value);
        glUniformMatrix4fv(location, 1, GL_FALSE, &value[0][0]);
        break;
      }
      case CommandType::SetUniformVec4: {
        i32 location;
        glm::vec4 value;
        read(location);
        read(value);
        glUniform4fv(location, 1, &value[0]);
        break;
      }
      case CommandType::SetUniformVec3: {
        i32 location;
        glm::vec3 value;
        read(location);
        read(value);
        glUniform3fv(location, 1, &value[0]);
        break;
      }
      case CommandType::SetUniformVec2: {
        i32 location;
        glm::vec2 value;
        read(location);
        read(value);
        glUniform2fv(location, 1, &value[0]);
        break;
      }
      case CommandType::SetUniformFloat: {
        i32 location;
        float value;
        read(location);
        read(value);
        glUniform1f(location, value);
        break;
      }
      case CommandType::SetUniformInt: {
        i32 location;
        i32 value;
        read(location);
        read(value);
        glUniform1i(location, value);
        break;
      }
      case CommandType::Draw: {
        u32 vertexCount, instanceCount, firstVertex, firstInstance;
        read(vertexCount);
        read(instanceCount);
        read(firstVertex);
        read(firstInstance);
        executeDraw(vertexCount, instanceCount, firstVertex, firstInstance);
        break;
      }
      case CommandType::DrawIndexed: {
        u32 indexCount, instanceCount, firstIndex, firstInstance;
        i32 vertexOffset;
        read(indexCount);
        read(instanceCount);
        read(firstIndex);
        read(vertexOffset);
        read(firstInstance);
        executeDrawIndexed(
          indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
        break;
      }
      case CommandType::PushDebugGroup: {
        u32 nameIndex;
        read(nameIndex);
#ifndef __EMSCRIPTEN__
        if (nameIndex < debugNames.size()) {
          glPushDebugGroup(
            GL_DEBUG_SOURCE_APPLICATION, 0, -1, debugNames[nameIndex].c_str());
        }
#endif
        break;
      }
      case CommandType::PopDebugGroup:
#ifndef __EMSCRIPTEN__
        glPopDebugGroup();
#endif
        break;
      case CommandType::BlitFramebuffer: {
        FramebufferId src, dst;
        i32 srcX0, srcY0, srcX1, srcY1;
        i32 dstX0, dstY0, dstX1, dstY1;
        bool colorBit, depthBit, stencilBit, linearFilter;
        read(src);
        read(dst);
        read(srcX0);
        read(srcY0);
        read(srcX1);
        read(srcY1);
        read(dstX0);
        read(dstY0);
        read(dstX1);
        read(dstY1);
        read(colorBit);
        read(depthBit);
        read(stencilBit);
        read(linearFilter);
        blitFramebuffer(src,
                        dst,
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
        break;
      }
    }
  }
}

void
Device::executeBeginRenderPass(const RenderPassBeginInfo& info)
{
  auto* fb = m_framebuffers.get(info.framebuffer);
  GLuint fboName = fb ? fb->glName : 0;

  if (m_stateCache.boundFBO != fboName) {
    glBindFramebuffer(GL_FRAMEBUFFER, fboName);
    m_stateCache.boundFBO = fboName;
  }

  if (info.clearColor) {
    for (u8 i = 0; i < info.colorAttachmentCount; ++i) {
      glClearBufferfv(GL_COLOR, i, &info.clearColors[i][0]);
    }
  }
  if (info.clearDepthStencil) {
    // Ensure depth writes are enabled — clears are masked by glDepthMask.
    GLboolean prevDepthMask;
    glGetBooleanv(GL_DEPTH_WRITEMASK, &prevDepthMask);
    if (!prevDepthMask)
      glDepthMask(GL_TRUE);

    glClearBufferfi(GL_DEPTH_STENCIL, 0, info.clearDepth, info.clearStencil);

    if (!prevDepthMask)
      glDepthMask(GL_FALSE);
  }
}

void
Device::executeEndRenderPass()
{
  // Nothing special needed for GLES3
}

void
Device::executeBindPipeline(PipelineId pipeline)
{
  auto* pipe = m_pipelines.get(pipeline);
  if (!pipe) {
    return;
  }

  auto* shader = m_shaders.get(pipe->program);
  if (shader && m_stateCache.boundProgram != shader->glName) {
    glUseProgram(shader->glName);
    m_stateCache.boundProgram = shader->glName;
  }

  // immediate mode passes may have changed VAO
  if (pipe->vao != 0 && m_stateCache.boundVAO != pipe->vao) {
    glBindVertexArray(pipe->vao);
    m_stateCache.boundVAO = pipe->vao;
  }
  // Reset explicit VAO tracking when pipeline binds its own VAO
  // (primitives may later bind their own VAOs via bindVertexArray command)
  m_stateCache.explicitVAO = {};

  if (m_stateCache.currentPipeline == pipeline) {
    return;
  }

  m_stateCache.currentPipeline = pipeline;

  if (pipe->depthStencil.depthTestEnable) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(toGLCompareFunc(pipe->depthStencil.depthCompareOp));
  } else {
    glDisable(GL_DEPTH_TEST);
  }
  glDepthMask(pipe->depthStencil.depthWriteEnable ? GL_TRUE : GL_FALSE);

  if (pipe->rasterizer.cullMode != CullMode::None) {
    glEnable(GL_CULL_FACE);
    glCullFace(pipe->rasterizer.cullMode == CullMode::Front ? GL_FRONT
                                                            : GL_BACK);
  } else {
    glDisable(GL_CULL_FACE);
  }
  glFrontFace(
    pipe->rasterizer.frontFace == FrontFace::CounterClockwise ? GL_CCW : GL_CW);

  // Blend state (first attachment only for simplicity)
  const auto& blendAtt = pipe->blend.attachments[0];
  if (blendAtt.blendEnable) {
    glEnable(GL_BLEND);
    glBlendFuncSeparate(toGLBlendFactor(blendAtt.srcColorBlendFactor),
                        toGLBlendFactor(blendAtt.dstColorBlendFactor),
                        toGLBlendFactor(blendAtt.srcAlphaBlendFactor),
                        toGLBlendFactor(blendAtt.dstAlphaBlendFactor));
    glBlendEquationSeparate(toGLBlendOp(blendAtt.colorBlendOp),
                            toGLBlendOp(blendAtt.alphaBlendOp));
  } else {
    glDisable(GL_BLEND);
  }
}

void
Device::executeSetViewport(const Viewport& viewport)
{
  glViewport(static_cast<GLint>(viewport.x),
             static_cast<GLint>(viewport.y),
             static_cast<GLsizei>(viewport.width),
             static_cast<GLsizei>(viewport.height));
  glDepthRangef(viewport.minDepth, viewport.maxDepth);
}

void
Device::executeSetScissor(const ScissorRect& scissor)
{
  glScissor(scissor.x,
            scissor.y,
            static_cast<GLsizei>(scissor.width),
            static_cast<GLsizei>(scissor.height));
}

void
Device::executeBindVertexBuffer(u32 binding, BufferId buffer, u64 offset)
{
  auto* res = m_buffers.get(buffer);
  if (!res) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, res->glName);

  // If an explicit VAO was bound via bindVertexArray command, use the VAO's
  // vertex layout instead of the pipeline's. The VAO already has vertex
  // attribute locations enabled; we just need to configure the pointers.
  auto* explicitVao = m_vertexArrays.get(m_stateCache.explicitVAO);
  if (explicitVao) {
    for (const auto& attrib : explicitVao->attributes) {
      if (attrib.binding == binding) {
        u32 stride = 0;
        bool perInstance = false;
        for (const auto& vaoBinding : explicitVao->bindings) {
          if (vaoBinding.binding == binding) {
            stride = vaoBinding.stride;
            perInstance = vaoBinding.perInstance;
            break;
          }
        }

        auto info = toGLVertexAttrib(attrib.format);
        if (info.isInteger) {
          glVertexAttribIPointer(
            attrib.location,
            info.size,
            info.type,
            static_cast<GLsizei>(stride),
            reinterpret_cast<const void*>(offset + attrib.offset));
        } else {
          glVertexAttribPointer(
            attrib.location,
            info.size,
            info.type,
            info.normalized,
            static_cast<GLsizei>(stride),
            reinterpret_cast<const void*>(offset + attrib.offset));
        }
        if (perInstance) {
          glVertexAttribDivisor(attrib.location, 1);
        } else {
          glVertexAttribDivisor(attrib.location, 0);
        }
      }
    }
    return;
  }

  // Fall back to pipeline's vertex layout
  auto* pipe = m_pipelines.get(m_stateCache.currentPipeline);
  if (!pipe) {
    return;
  }

  u32 stride = 0;
  bool perInstance = false;
  for (const auto& vb : pipe->vertexBindings) {
    if (vb.binding == binding) {
      stride = vb.stride;
      perInstance = vb.perInstance;
      break;
    }
  }

  for (const auto& attrib : pipe->vertexAttributes) {
    if (attrib.binding == binding) {
      auto info = toGLVertexAttrib(attrib.format);
      if (info.isInteger) {
        glVertexAttribIPointer(
          attrib.location,
          info.size,
          info.type,
          static_cast<GLsizei>(stride),
          reinterpret_cast<const void*>(offset + attrib.offset));
      } else {
        glVertexAttribPointer(
          attrib.location,
          info.size,
          info.type,
          info.normalized,
          static_cast<GLsizei>(stride),
          reinterpret_cast<const void*>(offset + attrib.offset));
      }
      if (perInstance) {
        glVertexAttribDivisor(attrib.location, 1);
      } else {
        glVertexAttribDivisor(attrib.location, 0);
      }
    }
  }
}

void
Device::executeBindIndexBuffer(BufferId buffer, u64 offset, IndexType indexType)
{
  auto* res = m_buffers.get(buffer);
  if (res) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, res->glName);
    m_boundIndexType = toGLIndexType(indexType);
    m_boundIndexBufferOffset = offset;
  }
}

void
Device::executeBindTexture(u32 slot, TextureId texture, SamplerId sampler)
{
  auto* tex = m_textures.get(texture);
  auto* samp = m_samplers.get(sampler);

  glActiveTexture(GL_TEXTURE0 + slot);

  if (tex) {
    glBindTexture(tex->glTarget, tex->glName);
    m_stateCache.boundTextures[slot] = tex->glName;
    m_stateCache.boundTextureTargets[slot] = tex->glTarget;
  }

  if (samp) {
    glBindSampler(slot, samp->glName);
    m_stateCache.boundSamplers[slot] = samp->glName;
  }
}

void
Device::executeDraw(u32 vertexCount,
                    u32 instanceCount,
                    u32 firstVertex,
                    u32 firstInstance)
{
  auto* pipe = m_pipelines.get(m_stateCache.currentPipeline);
  GLenum mode = pipe ? toGLPrimitive(pipe->topology) : GL_TRIANGLES;

  if (instanceCount > 1 || firstInstance > 0) {
#ifndef __EMSCRIPTEN__
    glDrawArraysInstancedBaseInstance(mode,
                                      static_cast<GLint>(firstVertex),
                                      static_cast<GLsizei>(vertexCount),
                                      static_cast<GLsizei>(instanceCount),
                                      firstInstance);
#else
    glDrawArraysInstanced(mode,
                          static_cast<GLint>(firstVertex),
                          static_cast<GLsizei>(vertexCount),
                          static_cast<GLsizei>(instanceCount));
#endif
  } else {
    glDrawArrays(
      mode, static_cast<GLint>(firstVertex), static_cast<GLsizei>(vertexCount));
  }
}

void
Device::executeDrawIndexed(u32 indexCount,
                           u32 instanceCount,
                           u32 firstIndex,
                           i32 /*vertexOffset*/,
                           u32 firstInstance)
{
  auto* pipe = m_pipelines.get(m_stateCache.currentPipeline);
  GLenum mode = pipe ? toGLPrimitive(pipe->topology) : GL_TRIANGLES;

  size_t indexSize = (m_boundIndexType == GL_UNSIGNED_INT) ? 4 : 2;
  const void* offset = reinterpret_cast<const void*>(m_boundIndexBufferOffset +
                                                     firstIndex * indexSize);

  if (instanceCount > 1 || firstInstance > 0) {
#ifndef __EMSCRIPTEN__
    glDrawElementsInstancedBaseInstance(mode,
                                        static_cast<GLsizei>(indexCount),
                                        m_boundIndexType,
                                        offset,
                                        static_cast<GLsizei>(instanceCount),
                                        firstInstance);
#else
    glDrawElementsInstanced(mode,
                            static_cast<GLsizei>(indexCount),
                            m_boundIndexType,
                            offset,
                            static_cast<GLsizei>(instanceCount));
#endif
  } else {
    glDrawElements(
      mode, static_cast<GLsizei>(indexCount), m_boundIndexType, offset);
  }
}

GLenum
Device::toGLInternalFormat(PixelFormat format)
{
  switch (format) {
    case PixelFormat::R8:
      return GL_R8;
    case PixelFormat::RG8:
      return GL_RG8;
    case PixelFormat::RGB8:
      return GL_RGB8;
    case PixelFormat::RGBA8:
      return GL_RGBA8;
    case PixelFormat::R16F:
      return GL_R16F;
    case PixelFormat::RG16F:
      return GL_RG16F;
    case PixelFormat::RGB16F:
      return GL_RGB16F;
    case PixelFormat::RGBA16F:
      return GL_RGBA16F;
    case PixelFormat::R32F:
      return GL_R32F;
    case PixelFormat::RG32F:
      return GL_RG32F;
    case PixelFormat::RGB32F:
      return GL_RGB32F;
    case PixelFormat::RGBA32F:
      return GL_RGBA32F;
    case PixelFormat::R11G11B10F:
      return GL_R11F_G11F_B10F;
    case PixelFormat::RGB10A2:
      return GL_RGB10_A2;
    case PixelFormat::Depth16:
      return GL_DEPTH_COMPONENT16;
    case PixelFormat::Depth24:
      return GL_DEPTH_COMPONENT24;
    case PixelFormat::Depth32F:
      return GL_DEPTH_COMPONENT32F;
    case PixelFormat::Depth24Stencil8:
      return GL_DEPTH24_STENCIL8;
    // Integer formats
    case PixelFormat::R8UI:
      return GL_R8UI;
    case PixelFormat::RG8UI:
      return GL_RG8UI;
    case PixelFormat::RGB8UI:
      return GL_RGB8UI;
    case PixelFormat::RGBA8UI:
      return GL_RGBA8UI;
    case PixelFormat::R16UI:
      return GL_R16UI;
    case PixelFormat::RG16UI:
      return GL_RG16UI;
    case PixelFormat::RGB16UI:
      return GL_RGB16UI;
    case PixelFormat::RGBA16UI:
      return GL_RGBA16UI;
    // Non-normalized 16-bit formats (used for vertex attributes)
    case PixelFormat::R16:
      return GL_R16UI;
    case PixelFormat::RG16:
      return GL_RG16UI;
    case PixelFormat::RGB16:
      return GL_RGB16UI;
    case PixelFormat::RGBA16:
      return GL_RGBA16UI;
    default:
      return GL_RGBA8;
  }
}

GLenum
Device::toGLFormat(PixelFormat format)
{
  switch (format) {
    case PixelFormat::R8:
    case PixelFormat::R16F:
    case PixelFormat::R32F:
      return GL_RED;
    case PixelFormat::RG8:
    case PixelFormat::RG16F:
    case PixelFormat::RG32F:
      return GL_RG;
    case PixelFormat::RGB8:
    case PixelFormat::RGB16F:
    case PixelFormat::RGB32F:
    case PixelFormat::R11G11B10F:
      return GL_RGB;
    case PixelFormat::RGBA8:
    case PixelFormat::RGBA16F:
    case PixelFormat::RGBA32F:
    case PixelFormat::RGB10A2:
      return GL_RGBA;
    case PixelFormat::Depth16:
    case PixelFormat::Depth24:
    case PixelFormat::Depth32F:
      return GL_DEPTH_COMPONENT;
    case PixelFormat::Depth24Stencil8:
      return GL_DEPTH_STENCIL;
    // Integer formats (use _INTEGER variants)
    case PixelFormat::R8UI:
    case PixelFormat::R16UI:
    case PixelFormat::R16:
      return GL_RED_INTEGER;
    case PixelFormat::RG8UI:
    case PixelFormat::RG16UI:
    case PixelFormat::RG16:
      return GL_RG_INTEGER;
    case PixelFormat::RGB8UI:
    case PixelFormat::RGB16UI:
    case PixelFormat::RGB16:
      return GL_RGB_INTEGER;
    case PixelFormat::RGBA8UI:
    case PixelFormat::RGBA16UI:
    case PixelFormat::RGBA16:
      return GL_RGBA_INTEGER;
    default:
      return GL_RGBA;
  }
}

GLenum
Device::toGLType(PixelFormat format)
{
  switch (format) {
    case PixelFormat::R8:
    case PixelFormat::RG8:
    case PixelFormat::RGB8:
    case PixelFormat::RGBA8:
      return GL_UNSIGNED_BYTE;
    case PixelFormat::R16F:
    case PixelFormat::RG16F:
    case PixelFormat::RGB16F:
    case PixelFormat::RGBA16F:
      return GL_HALF_FLOAT;
    case PixelFormat::R32F:
    case PixelFormat::RG32F:
    case PixelFormat::RGB32F:
    case PixelFormat::RGBA32F:
    case PixelFormat::Depth32F:
      return GL_FLOAT;
    case PixelFormat::R11G11B10F:
      return GL_UNSIGNED_INT_10F_11F_11F_REV;
    case PixelFormat::RGB10A2:
      return GL_UNSIGNED_INT_2_10_10_10_REV;
    case PixelFormat::Depth16:
      return GL_UNSIGNED_SHORT;
    case PixelFormat::Depth24:
      return GL_UNSIGNED_INT;
    case PixelFormat::Depth24Stencil8:
      return GL_UNSIGNED_INT_24_8;
    // Integer 8-bit formats
    case PixelFormat::R8UI:
    case PixelFormat::RG8UI:
    case PixelFormat::RGB8UI:
    case PixelFormat::RGBA8UI:
      return GL_UNSIGNED_BYTE;
    // Integer/non-normalized 16-bit formats
    case PixelFormat::R16UI:
    case PixelFormat::RG16UI:
    case PixelFormat::RGB16UI:
    case PixelFormat::RGBA16UI:
    case PixelFormat::R16:
    case PixelFormat::RG16:
    case PixelFormat::RGB16:
    case PixelFormat::RGBA16:
      return GL_UNSIGNED_SHORT;
    default:
      return GL_UNSIGNED_BYTE;
  }
}

GLenum
Device::toGLTarget(TextureType type)
{
  switch (type) {
    case TextureType::Texture2D:
      return GL_TEXTURE_2D;
    case TextureType::Texture2DArray:
      return GL_TEXTURE_2D_ARRAY;
    case TextureType::TextureCube:
      return GL_TEXTURE_CUBE_MAP;
    case TextureType::Texture3D:
      return GL_TEXTURE_3D;
    default:
      return GL_TEXTURE_2D;
  }
}

GLenum
Device::toGLBufferUsage(BufferUsage usage)
{
  if (hasFlag(usage, BufferUsage::Uniform))
    return GL_DYNAMIC_DRAW;
  return GL_STATIC_DRAW;
}

GLenum
Device::toGLPrimitive(PrimitiveTopology topology)
{
  switch (topology) {
    case PrimitiveTopology::Points:
      return GL_POINTS;
    case PrimitiveTopology::Lines:
      return GL_LINES;
    case PrimitiveTopology::LineStrip:
      return GL_LINE_STRIP;
    case PrimitiveTopology::Triangles:
      return GL_TRIANGLES;
    case PrimitiveTopology::TriangleStrip:
      return GL_TRIANGLE_STRIP;
    case PrimitiveTopology::TriangleFan:
      return GL_TRIANGLE_FAN;
    default:
      return GL_TRIANGLES;
  }
}

GLenum
Device::toGLCompareFunc(CompareOp op)
{
  switch (op) {
    case CompareOp::Never:
      return GL_NEVER;
    case CompareOp::Less:
      return GL_LESS;
    case CompareOp::Equal:
      return GL_EQUAL;
    case CompareOp::LessEqual:
      return GL_LEQUAL;
    case CompareOp::Greater:
      return GL_GREATER;
    case CompareOp::NotEqual:
      return GL_NOTEQUAL;
    case CompareOp::GreaterEqual:
      return GL_GEQUAL;
    case CompareOp::Always:
      return GL_ALWAYS;
    default:
      return GL_LESS;
  }
}

GLenum
Device::toGLBlendFactor(BlendFactor factor)
{
  switch (factor) {
    case BlendFactor::Zero:
      return GL_ZERO;
    case BlendFactor::One:
      return GL_ONE;
    case BlendFactor::SrcColor:
      return GL_SRC_COLOR;
    case BlendFactor::OneMinusSrcColor:
      return GL_ONE_MINUS_SRC_COLOR;
    case BlendFactor::DstColor:
      return GL_DST_COLOR;
    case BlendFactor::OneMinusDstColor:
      return GL_ONE_MINUS_DST_COLOR;
    case BlendFactor::SrcAlpha:
      return GL_SRC_ALPHA;
    case BlendFactor::OneMinusSrcAlpha:
      return GL_ONE_MINUS_SRC_ALPHA;
    case BlendFactor::DstAlpha:
      return GL_DST_ALPHA;
    case BlendFactor::OneMinusDstAlpha:
      return GL_ONE_MINUS_DST_ALPHA;
    case BlendFactor::ConstantColor:
      return GL_CONSTANT_COLOR;
    case BlendFactor::OneMinusConstantColor:
      return GL_ONE_MINUS_CONSTANT_COLOR;
    case BlendFactor::ConstantAlpha:
      return GL_CONSTANT_ALPHA;
    case BlendFactor::OneMinusConstantAlpha:
      return GL_ONE_MINUS_CONSTANT_ALPHA;
    case BlendFactor::SrcAlphaSaturate:
      return GL_SRC_ALPHA_SATURATE;
    default:
      return GL_ONE;
  }
}

GLenum
Device::toGLBlendOp(BlendOp op)
{
  switch (op) {
    case BlendOp::Add:
      return GL_FUNC_ADD;
    case BlendOp::Subtract:
      return GL_FUNC_SUBTRACT;
    case BlendOp::ReverseSubtract:
      return GL_FUNC_REVERSE_SUBTRACT;
    case BlendOp::Min:
      return GL_MIN;
    case BlendOp::Max:
      return GL_MAX;
    default:
      return GL_FUNC_ADD;
  }
}

GLenum
Device::toGLFilter(FilterMode mode)
{
  switch (mode) {
    case FilterMode::Nearest:
      return GL_NEAREST;
    case FilterMode::Linear:
      return GL_LINEAR;
    case FilterMode::NearestMipmapNearest:
      return GL_NEAREST_MIPMAP_NEAREST;
    case FilterMode::LinearMipmapNearest:
      return GL_LINEAR_MIPMAP_NEAREST;
    case FilterMode::NearestMipmapLinear:
      return GL_NEAREST_MIPMAP_LINEAR;
    case FilterMode::LinearMipmapLinear:
      return GL_LINEAR_MIPMAP_LINEAR;
    default:
      return GL_LINEAR;
  }
}

GLenum
Device::toGLWrap(WrapMode mode)
{
  switch (mode) {
    case WrapMode::Repeat:
      return GL_REPEAT;
    case WrapMode::MirroredRepeat:
      return GL_MIRRORED_REPEAT;
    case WrapMode::ClampToEdge:
      return GL_CLAMP_TO_EDGE;
    default:
      return GL_REPEAT;
  }
}

GLenum
Device::toGLIndexType(IndexType type)
{
  return (type == IndexType::U32) ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
}

GLenum
Device::toGLMagFilter(FilterMode mode)
{
  // Magnification filter cannot use mipmap modes - only GL_NEAREST or GL_LINEAR
  switch (mode) {
    case FilterMode::Nearest:
    case FilterMode::NearestMipmapNearest:
    case FilterMode::NearestMipmapLinear:
      return GL_NEAREST;
    case FilterMode::Linear:
    case FilterMode::LinearMipmapNearest:
    case FilterMode::LinearMipmapLinear:
    default:
      return GL_LINEAR;
  }
}

Device::VertexAttribInfo
Device::toGLVertexAttrib(PixelFormat format)
{
  switch (format) {
    // Single component formats (normalized)
    case PixelFormat::R8:
      return { 1, GL_UNSIGNED_BYTE, GL_TRUE, false };
    case PixelFormat::R16F:
      return { 1, GL_HALF_FLOAT, GL_FALSE, false };
    case PixelFormat::R32F:
      return { 1, GL_FLOAT, GL_FALSE, false };

    // Single component formats (unsigned integer)
    case PixelFormat::R8UI:
      return { 1, GL_UNSIGNED_BYTE, GL_FALSE, true };
    case PixelFormat::R16UI:
      return { 1, GL_UNSIGNED_SHORT, GL_FALSE, true };

    // Two component formats (normalized)
    case PixelFormat::RG8:
      return { 2, GL_UNSIGNED_BYTE, GL_TRUE, false };
    case PixelFormat::RG16F:
      return { 2, GL_HALF_FLOAT, GL_FALSE, false };
    case PixelFormat::RG32F:
      return { 2, GL_FLOAT, GL_FALSE, false };

    // Two component formats (unsigned integer)
    case PixelFormat::RG8UI:
      return { 2, GL_UNSIGNED_BYTE, GL_FALSE, true };
    case PixelFormat::RG16UI:
      return { 2, GL_UNSIGNED_SHORT, GL_FALSE, true };

    // Three component formats (normalized)
    case PixelFormat::RGB8:
      return { 3, GL_UNSIGNED_BYTE, GL_TRUE, false };
    case PixelFormat::RGB32F:
      return { 3, GL_FLOAT, GL_FALSE, false };

    // Three component formats (unsigned integer)
    case PixelFormat::RGB8UI:
      return { 3, GL_UNSIGNED_BYTE, GL_FALSE, true };
    case PixelFormat::RGB16UI:
      return { 3, GL_UNSIGNED_SHORT, GL_FALSE, true };

    // Four component formats (normalized)
    case PixelFormat::RGBA8:
      return { 4, GL_UNSIGNED_BYTE, GL_TRUE, false };
    case PixelFormat::RGBA16F:
      return { 4, GL_HALF_FLOAT, GL_FALSE, false };
    case PixelFormat::RGBA32F:
      return { 4, GL_FLOAT, GL_FALSE, false };
    case PixelFormat::RGB10A2:
      return { 4, GL_UNSIGNED_INT_2_10_10_10_REV, GL_TRUE, false };

    // Four component formats (unsigned integer)
    case PixelFormat::RGBA8UI:
      return { 4, GL_UNSIGNED_BYTE, GL_FALSE, true };
    case PixelFormat::RGBA16UI:
      return { 4, GL_UNSIGNED_SHORT, GL_FALSE, true };

    // 16-bit unsigned integer converted to float (non-normalized)
    // Used for attributes like JOINTS_0 where shader expects vec4
    case PixelFormat::R16:
      return { 1, GL_UNSIGNED_SHORT, GL_FALSE, false };
    case PixelFormat::RG16:
      return { 2, GL_UNSIGNED_SHORT, GL_FALSE, false };
    case PixelFormat::RGB16:
      return { 3, GL_UNSIGNED_SHORT, GL_FALSE, false };
    case PixelFormat::RGBA16:
      return { 4, GL_UNSIGNED_SHORT, GL_FALSE, false };

    default:
      return { 4, GL_FLOAT, GL_FALSE, false };
  }
}

// Render state management (immediate mode, for migration from raw GL)

void
Device::bindFramebuffer(FramebufferId fbo)
{
  GLuint glFbo = 0;
  if (fbo.isValid()) {
    auto* res = m_framebuffers.get(fbo);
    if (res) {
      glFbo = res->glName;
    }
  }

  if (m_stateCache.boundFBO != glFbo) {
    glBindFramebuffer(GL_FRAMEBUFFER, glFbo);
    m_stateCache.boundFBO = glFbo;
  }
}

void
Device::setViewport(i32 x, i32 y, u32 width, u32 height)
{
  glViewport(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void
Device::setScissor(i32 x, i32 y, u32 width, u32 height)
{
  glScissor(x, y, static_cast<GLsizei>(width), static_cast<GLsizei>(height));
}

void
Device::clearColor(std::optional<u32> attachment, const glm::vec4& color)
{
  // In OpenGL ES 3.0, glClearBufferfv clears specific draw buffers
  // nullopt means clear all (use legacy glClear with glClearColor)
  if (!attachment) {
    glClearColor(color.r, color.g, color.b, color.a);
    glClear(GL_COLOR_BUFFER_BIT);
  } else {
    glClearBufferfv(GL_COLOR, *attachment, glm::value_ptr(color));
  }
}

void
Device::clearDepth(float depth)
{
  // Ensure depth writes are enabled — glClear is a no-op when depth mask is
  // GL_FALSE
  GLboolean depthMask;
  glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
  if (!depthMask)
    glDepthMask(GL_TRUE);

  glClearDepthf(depth);
  glClear(GL_DEPTH_BUFFER_BIT);

  if (!depthMask)
    glDepthMask(GL_FALSE);
}

void
Device::clearStencil(i32 value)
{
  glClearStencil(value);
  glClear(GL_STENCIL_BUFFER_BIT);
}

void
Device::clearDepthStencil(float depth, i32 stencil)
{
  glClearDepthf(depth);
  glClearStencil(stencil);
  glClear(GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void
Device::setDepthTest(bool enable)
{
  if (enable) {
    glEnable(GL_DEPTH_TEST);
  } else {
    glDisable(GL_DEPTH_TEST);
  }
}

void
Device::setDepthFunc(CompareOp op)
{
  glDepthFunc(toGLCompareFunc(op));
}

void
Device::setDepthWrite(bool enable)
{
  glDepthMask(enable ? GL_TRUE : GL_FALSE);
}

void
Device::setCullMode(CullMode mode)
{
  switch (mode) {
    case CullMode::None:
      glDisable(GL_CULL_FACE);
      break;
    case CullMode::Front:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_FRONT);
      break;
    case CullMode::Back:
      glEnable(GL_CULL_FACE);
      glCullFace(GL_BACK);
      break;
  }
}

void
Device::setFrontFace(FrontFace face)
{
  glFrontFace(face == FrontFace::CounterClockwise ? GL_CCW : GL_CW);
}

void
Device::setBlendEnabled(bool enable)
{
  if (enable) {
    glEnable(GL_BLEND);
  } else {
    glDisable(GL_BLEND);
  }
}

void
Device::setBlendFunc(BlendFactor srcColor,
                     BlendFactor dstColor,
                     BlendFactor srcAlpha,
                     BlendFactor dstAlpha)
{
  glBlendFuncSeparate(toGLBlendFactor(srcColor),
                      toGLBlendFactor(dstColor),
                      toGLBlendFactor(srcAlpha),
                      toGLBlendFactor(dstAlpha));
}

void
Device::setBlendEquation(BlendOp colorOp, BlendOp alphaOp)
{
  glBlendEquationSeparate(toGLBlendOp(colorOp), toGLBlendOp(alphaOp));
}

void
Device::setScissorTest(bool enable)
{
  if (enable) {
    glEnable(GL_SCISSOR_TEST);
  } else {
    glDisable(GL_SCISSOR_TEST);
  }
}

void
Device::bindShaderProgram(ShaderId program)
{
  GLuint glProgram = 0;
  if (program.isValid()) {
    auto* res = m_shaders.get(program);
    if (res) {
      glProgram = res->glName;
    }
  }

  if (m_stateCache.boundProgram != glProgram) {
    glUseProgram(glProgram);
    m_stateCache.boundProgram = glProgram;
  }
}

void
Device::bindTexture(u32 unit, TextureId texture)
{
  if (unit >= 16) {
    return;
  }

  GLuint glTex = 0;
  GLenum target = GL_TEXTURE_2D;
  if (texture.isValid()) {
    auto* res = m_textures.get(texture);
    if (res) {
      glTex = res->glName;
      target = res->glTarget;
    }
  }

  // Always bind — internal helpers (createTexture, updateTexture, generateMipmaps)
  // call glBindTexture directly without updating the cache, so skipping here
  // would leave stale bindings.
  glActiveTexture(GL_TEXTURE0 + unit);
  glBindTexture(target, glTex);
  m_stateCache.boundTextures[unit] = glTex;
  m_stateCache.boundTextureTargets[unit] = target;
}

void
Device::bindSampler(u32 unit, SamplerId sampler)
{
  if (unit >= 16) {
    return;
  }

  GLuint glSampler = 0;
  if (sampler.isValid()) {
    auto* res = m_samplers.get(sampler);
    if (res) {
      glSampler = res->glName;
    }
  }

  if (m_stateCache.boundSamplers[unit] != glSampler) {
    glBindSampler(unit, glSampler);
    m_stateCache.boundSamplers[unit] = glSampler;
  }
}

void
Device::blitFramebuffer(FramebufferId srcFbo,
                        FramebufferId dstFbo,
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
  GLuint srcGl = 0;
  GLuint dstGl = 0;

  if (srcFbo.isValid()) {
    auto* res = m_framebuffers.get(srcFbo);
    if (res) {
      srcGl = res->glName;
    }
  }

  if (dstFbo.isValid()) {
    auto* res = m_framebuffers.get(dstFbo);
    if (res) {
      dstGl = res->glName;
    }
  }

  glBindFramebuffer(GL_READ_FRAMEBUFFER, srcGl);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, dstGl);

  GLbitfield mask = 0;
  if (colorBit)
    mask |= GL_COLOR_BUFFER_BIT;
  if (depthBit)
    mask |= GL_DEPTH_BUFFER_BIT;
  if (stencilBit)
    mask |= GL_STENCIL_BUFFER_BIT;

  // Linear filtering only valid for color (depth/stencil must be nearest)
  GLenum filter = linearFilter ? GL_LINEAR : GL_NEAREST;

  glBlitFramebuffer(
    srcX0, srcY0, srcX1, srcY1, dstX0, dstY0, dstX1, dstY1, mask, filter);

  // Restore FBO binding state (invalidate cache since we changed both)
  m_stateCache.boundFBO = 0;
}

void
Device::setSeamlessCubemap(bool enable)
{
#if !defined(__EMSCRIPTEN__)
  if (enable) {
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  } else {
    glDisable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
  }
#else
  (void)enable;
#endif
}

void
Device::setLineWidth(float width)
{
  glLineWidth(width);
}

void
Device::setLineSmoothing(bool enable)
{
#if !defined(__EMSCRIPTEN__)
  if (enable) {
    glEnable(GL_LINE_SMOOTH);
  } else {
    glDisable(GL_LINE_SMOOTH);
  }
#else
  (void)enable;
#endif
}

void
Device::setColorMask(bool red, bool green, bool blue, bool alpha)
{
  glColorMask(red ? GL_TRUE : GL_FALSE,
              green ? GL_TRUE : GL_FALSE,
              blue ? GL_TRUE : GL_FALSE,
              alpha ? GL_TRUE : GL_FALSE);
}

void
Device::setBlendColor(float red, float green, float blue, float alpha)
{
  glBlendColor(red, green, blue, alpha);
}

void
Device::setReadBuffer(std::optional<u32> attachment)
{
  if (!attachment) {
    glReadBuffer(GL_NONE);
  } else {
    glReadBuffer(GL_COLOR_ATTACHMENT0 + *attachment);
  }
}

void
Device::bindUniformBuffer(u32 bindingPoint,
                          BufferId buffer,
                          u64 offset,
                          u64 size)
{
  auto* res = m_buffers.get(buffer);
  if (!res) {
    return;
  }

  glBindBufferRange(GL_UNIFORM_BUFFER,
                    bindingPoint,
                    res->glName,
                    static_cast<GLintptr>(offset),
                    static_cast<GLsizeiptr>(size));
}

void
Device::bindUniformBlock(ShaderId program,
                         const char* blockName,
                         u32 bindingPoint)
{
  auto* res = m_shaders.get(program);
  if (!res || !res->isLinkedProgram) {
    return;
  }

  GLuint blockIndex = glGetUniformBlockIndex(res->glName, blockName);
  if (blockIndex != GL_INVALID_INDEX) {
    glUniformBlockBinding(res->glName, blockIndex, bindingPoint);
  }
}

} // namespace gfx::gles3
