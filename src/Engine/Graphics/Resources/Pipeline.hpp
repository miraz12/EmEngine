#pragma once

#include "../GraphicsTypes.hpp"
#include "../Handle.hpp"
#include <array>
#include <span>

namespace gfx {

/// Vertex attribute description
struct VertexAttribute
{
  u32 location{ 0 }; ///< Shader attribute location
  u32 binding{ 0 };  ///< Which vertex buffer binding this reads from
  u32 offset{ 0 };   ///< Offset within the vertex
  PixelFormat format{ PixelFormat::Unknown };
};

/// Vertex binding description
struct VertexBinding
{
  u32 binding{ 0 };
  u32 stride{ 0 };
  bool perInstance{ false };
};

/// Blend state for a single render target
struct BlendAttachmentState
{
  bool blendEnable{ false };
  BlendFactor srcColorBlendFactor{ BlendFactor::One };
  BlendFactor dstColorBlendFactor{ BlendFactor::Zero };
  BlendOp colorBlendOp{ BlendOp::Add };
  BlendFactor srcAlphaBlendFactor{ BlendFactor::One };
  BlendFactor dstAlphaBlendFactor{ BlendFactor::Zero };
  BlendOp alphaBlendOp{ BlendOp::Add };
  u8 colorWriteMask{ 0xF }; // RGBA bits
};

/// Complete blend state
struct BlendState
{
  static constexpr u32 kMaxColorAttachments = 8;
  std::array<BlendAttachmentState, kMaxColorAttachments> attachments{};
  glm::vec4 blendConstants{ 0.0f, 0.0f, 0.0f, 0.0f };
};

/// Depth and stencil testing state
struct DepthStencilState
{
  bool depthTestEnable{ true };
  bool depthWriteEnable{ true };
  CompareOp depthCompareOp{ CompareOp::Less };
  bool stencilTestEnable{ false };
};

/// Rasterizer state
struct RasterizerState
{
  CullMode cullMode{ CullMode::Back };
  FrontFace frontFace{ FrontFace::CounterClockwise };
  float lineWidth{ 1.0f };
};

/// Pipeline creation descriptor
struct PipelineCreateInfo
{
  ShaderId shaderProgram;
  std::span<const VertexBinding> vertexBindings;
  std::span<const VertexAttribute> vertexAttributes;
  PrimitiveTopology topology{ PrimitiveTopology::Triangles };
  RasterizerState rasterizer;
  DepthStencilState depthStencil;
  BlendState blend;
  u32 colorAttachmentCount{ 1 };
  std::array<PixelFormat, BlendState::kMaxColorAttachments> colorFormats{};
  PixelFormat depthFormat{ PixelFormat::Unknown };
  const char* debugName{ nullptr };
};

/// Vertex array (VAO) creation descriptor
struct VertexArrayCreateInfo
{
  std::span<const VertexBinding> vertexBindings;
  std::span<const VertexAttribute> vertexAttributes;
  PrimitiveTopology topology{ PrimitiveTopology::Triangles };
  u32 vertexCount{ 0 };
  const char* debugName{ nullptr };
};

} // namespace gfx
