#pragma once

#include "../GraphicsTypes.hpp"
#include "../Handle.hpp"

namespace gfx {

/// Shader stage creation descriptor (vertex or fragment)
struct ShaderCreateInfo
{
  ShaderStage stage{ ShaderStage::Vertex };
  const char* source{ nullptr };
  size_t sourceLength{ 0 };
  const char* debugName{ nullptr };
};

/// Shader program creation descriptor (links vertex + fragment)
struct ShaderProgramCreateInfo
{
  ShaderId vertexShader;
  ShaderId fragmentShader;
  const char* debugName{ nullptr };
};

} // namespace gfx
