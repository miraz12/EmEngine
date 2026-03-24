#pragma once

#include "../GraphicsTypes.hpp"

namespace gfx {

/// Buffer creation descriptor
struct BufferCreateInfo
{
  u64 size{ 0 };
  BufferUsage usage{ BufferUsage::None };
  const void* initialData{ nullptr };
  const char* debugName{ nullptr };
};

} // namespace gfx
