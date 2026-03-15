#ifndef LIGHTINGUTIL_H_
#define LIGHTINGUTIL_H_

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

} // namespace LightingUtil

#endif // LIGHTINGUTIL_H_
