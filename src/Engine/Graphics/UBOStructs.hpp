#pragma once

#include "engine_pch.hpp"

namespace gfx {

/// Standard UBO binding points.
enum class UBOBinding : u32
{
  CascadeData = 0, // Shadow cascade matrices (existing)
  Camera = 1,      // View/projection matrices
  Lighting = 2,    // Directional + point lights
  Material = 3,    // PBR material properties
  Transform = 4,   // Model/normal matrices
  PostProcess = 5, // Post-processing parameters
};

/// Camera matrices - shared across all passes that need view/projection.
/// std140 layout - 208 bytes
struct alignas(16) CameraUBO
{
  glm::mat4 viewMatrix;     // offset: 0
  glm::mat4 projMatrix;     // offset: 64
  glm::mat4 viewProjMatrix; // offset: 128
  glm::vec4 cameraPosition; // offset: 192 (xyz = pos, w = unused)
};
static_assert(sizeof(CameraUBO) == 208, "CameraUBO size mismatch");

/// Point light data for the lighting UBO.
/// std140 layout - 48 bytes
struct alignas(16) PointLightData
{
  glm::vec4 positionRadius; // xyz = position, w = radius
  glm::vec4 colorIntensity; // xyz = color, w = unused
  glm::vec4 attenuation; // x = constant, y = linear, z = quadratic, w = unused
};
static_assert(sizeof(PointLightData) == 48, "PointLightData size mismatch");

/// Lighting data - directional light + point lights.
/// std140 layout - 528 bytes
struct alignas(16) LightingUBO
{
  glm::vec4 dirLightDirection;    // xyz = direction, w = unused
  glm::vec4 dirLightColor;        // xyz = color, w = intensity
  PointLightData pointLights[10]; // 480 bytes (10 * 48)
  glm::ivec4 lightConfig; // x = numPointLights, y = debugView, zw = unused
};
static_assert(sizeof(LightingUBO) == 528, "LightingUBO size mismatch");

/// PBR material properties.
/// std140 layout - 64 bytes
struct alignas(16) MaterialUBO
{
  glm::vec4 baseColorFactor; // xyz = color, w = alpha
  glm::vec4 emissiveFactor;  // xyz = emissive, w = unused
  glm::vec4
    pbrFactors; // x = roughness, y = metallic, z = alphaCutoff, w = unused
  glm::ivec4 materialConfig; // x = materialFlags, y = alphaMode, zw = unused
};
static_assert(sizeof(MaterialUBO) == 64, "MaterialUBO size mismatch");

/// Per-object transform matrices.
/// std140 layout - 192 bytes
struct alignas(16) TransformUBO
{
  glm::mat4 modelMatrix;  // offset: 0
  glm::mat4 normalMatrix; // offset: 64 (mat3 expanded to mat4 for std140)
  glm::mat4 meshMatrix;   // offset: 128
};
static_assert(sizeof(TransformUBO) == 192, "TransformUBO size mismatch");

/// Post-processing parameters (bloom, FXAA, etc.).
/// std140 layout - 32 bytes
struct alignas(16) PostProcessUBO
{
  glm::vec4 resolution; // xy = resolution, z = exposure, w = filterRadius
  glm::ivec4
    postConfig; // x = mipLevel, y = sceneSampler, z = bloomSampler, w = unused
};
static_assert(sizeof(PostProcessUBO) == 32, "PostProcessUBO size mismatch");

} // namespace gfx
