#pragma once

#include "../GraphicsTypes.hpp"
#include "../Handle.hpp"

namespace gfx {

/// Renderbuffer creation descriptor
/// Renderbuffers are write-only attachments optimized for depth/stencil
struct RenderbufferCreateInfo
{
  u32 width{ 0 };
  u32 height{ 0 };
  PixelFormat format{ PixelFormat::Depth24Stencil8 };
  const char* debugName{ nullptr };
};

} // namespace gfx
