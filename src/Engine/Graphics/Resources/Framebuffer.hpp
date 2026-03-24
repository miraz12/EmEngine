#pragma once

#include "../GraphicsTypes.hpp"
#include "../Handle.hpp"
#include <span>

namespace gfx {

/// Framebuffer attachment descriptor
struct FramebufferAttachment
{
  TextureId texture;
  u32 mipLevel{ 0 };
  u32 layer{ 0 }; // For array/cube textures
};

/// Framebuffer creation descriptor
struct FramebufferCreateInfo
{
  std::span<const FramebufferAttachment> colorAttachments;
  FramebufferAttachment depthStencilAttachment;
  u32 width{ 0 };
  u32 height{ 0 };
  const char* debugName{ nullptr };
};

} // namespace gfx
