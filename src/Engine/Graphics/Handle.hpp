#pragma once

#include "engine_pch.hpp"

namespace gfx {

/// Type-safe opaque handle with generation counter for safe GPU resource
/// management. Uses 24-bit index + 8-bit generation to detect stale handle
/// usage.
template<typename Tag>
struct Handle
{
  u32 value{ 0 };

  static constexpr u32 kIndexBits = 24;
  static constexpr u32 kGenBits = 8;
  static constexpr u32 kIndexMask = (1u << kIndexBits) - 1;
  static constexpr u32 kGenMask = (1u << kGenBits) - 1;

  [[nodiscard]] u32 index() const { return value & kIndexMask; }
  [[nodiscard]] u8 generation() const
  {
    return static_cast<u8>((value >> kIndexBits) & kGenMask);
  }
  [[nodiscard]] bool isValid() const { return value != 0; }

  static Handle create(u32 idx, u8 gen)
  {
    return Handle{ (idx & kIndexMask) |
                   ((static_cast<u32>(gen) & kGenMask) << kIndexBits) };
  }

  bool operator==(const Handle& other) const { return value == other.value; }
};

// Type tags for compile-time handle safety
struct BufferTag
{};
struct TextureTag
{};
struct ShaderTag
{};
struct PipelineTag
{};
struct FramebufferTag
{};
struct RenderbufferTag
{};
struct SamplerTag
{};
struct CommandBufferTag
{};
struct VertexArrayTag
{};

using BufferId = Handle<BufferTag>;
using TextureId = Handle<TextureTag>;
using ShaderId = Handle<ShaderTag>;
using PipelineId = Handle<PipelineTag>;
using FramebufferId = Handle<FramebufferTag>;
using RenderbufferId = Handle<RenderbufferTag>;
using SamplerId = Handle<SamplerTag>;
using CommandBufferId = Handle<CommandBufferTag>;
using VertexArrayId = Handle<VertexArrayTag>;

} // namespace gfx

template<typename Tag>
struct std::hash<gfx::Handle<Tag>>
{
  std::size_t operator()(const gfx::Handle<Tag>& h) const noexcept
  {
    return std::hash<u32>{}(h.value);
  }
};
