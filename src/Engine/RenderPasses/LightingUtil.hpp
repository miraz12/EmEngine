#ifndef LIGHTINGUTIL_H_
#define LIGHTINGUTIL_H_

#include <array>
#include <cfloat>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace LightingUtil {

inline glm::mat4
calculateLightSpaceMatrix(const glm::vec3& lightDirection,
                          const glm::vec3& cameraPosition)
{
  constexpr float shadowBox = 50.0f;     // 100x100 unit coverage
  constexpr float lightDistance = 50.0f; // Distance from frustum center
  constexpr float nearPlane = 0.1f;
  constexpr float farPlane = 150.0f;

  glm::mat4 lightProjection = glm::ortho(
    -shadowBox, shadowBox, -shadowBox, shadowBox, nearPlane, farPlane);

  // Normalize light direction to ensure consistent behavior
  glm::vec3 normLightDir = glm::normalize(lightDirection);

  // Center the shadow map frustum on the camera to ensure proper coverage
  glm::vec3 frustumCenter = cameraPosition;

  // Position light along the negative light direction (opposite to where light
  // points)
  glm::vec3 lightPos = frustumCenter - normLightDir * lightDistance;

  // Choose an up vector that's not parallel to the light direction
  // If light is pointing mostly up/down, use Z-axis as up, otherwise use Y-axis
  glm::vec3 up = (glm::abs(normLightDir.y) > 0.99f) ? glm::vec3(0, 0, 1)
                                                    : glm::vec3(0, 1, 0);

  glm::mat4 lightView = glm::lookAt(lightPos, frustumCenter, up);

  return lightProjection * lightView;
}

struct CascadeConfig
{
  static constexpr u32 NUM_CASCADES = 4;
  static constexpr u32 SHADOW_MAP_SIZE = 1024;

  glm::mat4 lightSpaceMatrices[NUM_CASCADES];
  std::array<float, NUM_CASCADES> cascadeSplits;

  // Calculate cascade split distances using PSSM (Parallel-Split Shadow Maps)
  inline void calculateSplitDistances(float nearPlane, float farPlane,
                                      float lambda = 0.5f)
  {
    float range = farPlane - nearPlane;
    float ratio = farPlane / nearPlane;

    for (u32 i = 0; i < NUM_CASCADES; ++i) {
      float p = static_cast<float>(i + 1) / static_cast<float>(NUM_CASCADES);
      float logSplit = nearPlane * std::pow(ratio, p);
      float uniformSplit = nearPlane + range * p;
      cascadeSplits[i] = lambda * logSplit + (1.0f - lambda) * uniformSplit;
    }
  }

  // Calculate tight-fitting orthographic projection for each cascade
  inline void calculateCascadeMatrices(const glm::vec3& lightDirection,
                                       const glm::vec3& cameraPosition,
                                       const glm::mat4& viewMatrix,
                                       const glm::mat4& projectionMatrix)
  {
    glm::vec3 lightDir = glm::normalize(lightDirection);

    for (u32 cascade = 0; cascade < NUM_CASCADES; ++cascade) {
      float nearClip = (cascade == 0) ? 0.1f : cascadeSplits[cascade - 1];
      float farClip = cascadeSplits[cascade];

      // Create projection matrix for this cascade frustum
      glm::mat4 cascadeProj = projectionMatrix;
      cascadeProj[2][2] = -(farClip + nearClip) / (farClip - nearClip);
      cascadeProj[3][2] = -(2.0f * farClip * nearClip) / (farClip - nearClip);

      glm::mat4 invViewProj = glm::inverse(cascadeProj * viewMatrix);

      // Extract frustum corners in NDC space [-1, 1]
      std::array<glm::vec3, 8> frustumCorners = {
        // Near face
        glm::vec3(-1.0f, -1.0f, -1.0f),
        glm::vec3(1.0f, -1.0f, -1.0f),
        glm::vec3(-1.0f, 1.0f, -1.0f),
        glm::vec3(1.0f, 1.0f, -1.0f),
        // Far face
        glm::vec3(-1.0f, -1.0f, 1.0f),
        glm::vec3(1.0f, -1.0f, 1.0f),
        glm::vec3(-1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 1.0f, 1.0f)
      };

      // Transform frustum corners to world space
      for (auto& corner : frustumCorners) {
        glm::vec4 worldCorner = invViewProj * glm::vec4(corner, 1.0f);
        corner = glm::vec3(worldCorner) / worldCorner.w;
      }

      // Compute frustum center in world space
      glm::vec3 center = glm::vec3(0.0f);
      for (const auto& corner : frustumCorners) {
        center += corner;
      }
      center /= 8.0f;

      // Create light view matrix looking at frustum center
      glm::vec3 up = (std::abs(lightDir.y) > 0.99f) ? glm::vec3(0.0f, 0.0f, 1.0f)
                                                     : glm::vec3(0.0f, 1.0f, 0.0f);
      glm::mat4 lightView =
        glm::lookAt(center - lightDir * 50.0f, center, up);

      // Transform frustum corners to light space to get tight bounds
      glm::vec3 lightMin(FLT_MAX);
      glm::vec3 lightMax(-FLT_MAX);

      for (const auto& corner : frustumCorners) {
        glm::vec4 lightCorner = lightView * glm::vec4(corner, 1.0f);
        lightMin = glm::min(lightMin, glm::vec3(lightCorner));
        lightMax = glm::max(lightMax, glm::vec3(lightCorner));
      }

      // Extend Z range to capture occluders behind the frustum
      lightMin.z -= 50.0f;
      lightMax.z += 10.0f;

      // Texel snapping for shadow stability (prevents swimming)
      float worldUnitsPerTexel =
        (lightMax.x - lightMin.x) / static_cast<float>(SHADOW_MAP_SIZE);
      lightMin.x = std::floor(lightMin.x / worldUnitsPerTexel) * worldUnitsPerTexel;
      lightMin.y = std::floor(lightMin.y / worldUnitsPerTexel) * worldUnitsPerTexel;
      lightMax.x = std::floor(lightMax.x / worldUnitsPerTexel) * worldUnitsPerTexel;
      lightMax.y = std::floor(lightMax.y / worldUnitsPerTexel) * worldUnitsPerTexel;

      // Create orthographic projection for this cascade
      glm::mat4 lightProj = glm::ortho(lightMin.x, lightMax.x, lightMin.y,
                                       lightMax.y, 0.0f, lightMax.z - lightMin.z);

      lightSpaceMatrices[cascade] = lightProj * lightView;
    }
  }
};

} // namespace LightingUtil

#endif // LIGHTINGUTIL_H_
