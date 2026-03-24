#pragma once

#include "../GraphicsTypes.hpp"

namespace gfx {

/// Sampler creation descriptor
struct SamplerCreateInfo
{
  FilterMode minFilter{ FilterMode::Linear };
  FilterMode magFilter{ FilterMode::Linear };
  WrapMode wrapU{ WrapMode::Repeat };
  WrapMode wrapV{ WrapMode::Repeat };
  WrapMode wrapW{ WrapMode::Repeat };
  float maxAnisotropy{ 1.0f };
  CompareOp compareOp{ CompareOp::Never };
  bool compareEnable{ false };
  const char* debugName{ nullptr };
};

} // namespace gfx
