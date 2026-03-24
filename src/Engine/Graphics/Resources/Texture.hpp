#pragma once

#include "../GraphicsTypes.hpp"

namespace gfx {

/// Texture creation descriptor
struct TextureCreateInfo
{
  TextureType type{ TextureType::Texture2D };
  PixelFormat format{ PixelFormat::RGBA8 };
  u32 width{ 1 };
  u32 height{ 1 };
  u32 depthOrLayers{ 1 };
  u32 mipLevels{ 1 };
  TextureUsage usage{ TextureUsage::Sampled };
  TextureStorageMode storageMode{ TextureStorageMode::Immutable };
  const void* initialData{ nullptr };
  const char* debugName{ nullptr };
};

} // namespace gfx
