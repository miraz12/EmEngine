#pragma once

#include "engine_pch.hpp"

namespace gfx {

/// Pixel formats (GLES3/WebGL2 compatible subset)
enum class PixelFormat : u8
{
  Unknown,
  // 8-bit normalized
  R8,
  RG8,
  RGB8,
  RGBA8,
  // 8-bit unsigned integer (for vertex attributes like JOINTS_0)
  R8UI,
  RG8UI,
  RGB8UI,
  RGBA8UI,
  // 16-bit unsigned integer (for vertex attributes)
  R16UI,
  RG16UI,
  RGB16UI,
  RGBA16UI,
  // 16-bit unsigned integer converted to float (for attributes like JOINTS_0
  // where the shader expects vec4 but data is stored as u16)
  R16,
  RG16,
  RGB16,
  RGBA16,
  // 16-bit float
  R16F,
  RG16F,
  RGB16F,
  RGBA16F,
  // 32-bit float
  R32F,
  RG32F,
  RGB32F,
  RGBA32F,
  // Packed formats
  R11G11B10F,
  RGB10A2,
  // Depth/stencil
  Depth16,
  Depth24,
  Depth32F,
  Depth24Stencil8,
};

/// Buffer usage flags (bitfield)
enum class BufferUsage : u16
{
  None = 0,
  Vertex = 1 << 0,
  Index = 1 << 1,
  Uniform = 1 << 2,
  TransferSrc = 1 << 3,
  TransferDst = 1 << 4,
};

inline BufferUsage
operator|(BufferUsage lhs, BufferUsage rhs)
{
  return static_cast<BufferUsage>(static_cast<u16>(lhs) |
                                  static_cast<u16>(rhs));
}

inline BufferUsage
operator&(BufferUsage lhs, BufferUsage rhs)
{
  return static_cast<BufferUsage>(static_cast<u16>(lhs) &
                                  static_cast<u16>(rhs));
}

inline bool
hasFlag(BufferUsage flags, BufferUsage flag)
{
  return (static_cast<u16>(flags) & static_cast<u16>(flag)) != 0;
}

/// Texture storage allocation strategy
enum class TextureStorageMode : u8
{
  Immutable, // glTexStorage2D - cannot resize (default)
  Mutable,   // glTexImage2D - can resize via resizeTexture
};

/// Texture types
enum class TextureType : u8
{
  Texture2D,
  Texture2DArray,
  TextureCube,
  Texture3D,
};

/// Texture usage flags (bitfield)
enum class TextureUsage : u8
{
  None = 0,
  Sampled = 1 << 0,
  RenderTarget = 1 << 1,
};

inline TextureUsage
operator|(TextureUsage lhs, TextureUsage rhs)
{
  return static_cast<TextureUsage>(static_cast<u8>(lhs) | static_cast<u8>(rhs));
}

inline TextureUsage
operator&(TextureUsage lhs, TextureUsage rhs)
{
  return static_cast<TextureUsage>(static_cast<u8>(lhs) & static_cast<u8>(rhs));
}

inline bool
hasFlag(TextureUsage flags, TextureUsage flag)
{
  return (static_cast<u8>(flags) & static_cast<u8>(flag)) != 0;
}

/// Primitive topology for draw calls
enum class PrimitiveTopology : u8
{
  Points,
  Lines,
  LineStrip,
  Triangles,
  TriangleStrip,
  TriangleFan,
};

/// Comparison operations (depth test, stencil, etc.)
enum class CompareOp : u8
{
  Never,
  Less,
  Equal,
  LessEqual,
  Greater,
  NotEqual,
  GreaterEqual,
  Always,
};

/// Blend factors
enum class BlendFactor : u8
{
  Zero,
  One,
  SrcColor,
  OneMinusSrcColor,
  DstColor,
  OneMinusDstColor,
  SrcAlpha,
  OneMinusSrcAlpha,
  DstAlpha,
  OneMinusDstAlpha,
  ConstantColor,
  OneMinusConstantColor,
  ConstantAlpha,
  OneMinusConstantAlpha,
  SrcAlphaSaturate,
};

/// Blend operations
enum class BlendOp : u8
{
  Add,
  Subtract,
  ReverseSubtract,
  Min,
  Max,
};

/// Face culling mode
enum class CullMode : u8
{
  None,
  Front,
  Back,
};

/// Front face winding order
enum class FrontFace : u8
{
  CounterClockwise,
  Clockwise,
};

/// Texture filtering modes
enum class FilterMode : u8
{
  Nearest,
  Linear,
  NearestMipmapNearest,
  LinearMipmapNearest,
  NearestMipmapLinear,
  LinearMipmapLinear,
};

/// Texture wrap/address modes
enum class WrapMode : u8
{
  Repeat,
  MirroredRepeat,
  ClampToEdge,
};

/// Shader stages
enum class ShaderStage : u8
{
  Vertex,
  Fragment,
};

/// Index buffer element type
enum class IndexType : u8
{
  U16,
  U32,
};

enum class RenderbufferAttachment
{
  Depth,
  Stencil,
  DepthStencil,
};

} // namespace gfx
