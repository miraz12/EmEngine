#ifndef LIGHTINGUTIL_H_
#define LIGHTINGUTIL_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace LightingUtil {

inline glm::mat4
calculateLightSpaceMatrix(const glm::vec3& lightDirection,
                          const glm::vec3& cameraPosition)
{
  constexpr float shadowBox = 25.0f; // 50x50 unit coverage
  glm::mat4 lightProjection =
    glm::ortho(-shadowBox, shadowBox, -shadowBox, shadowBox, 1.0f, 50.0f);

  glm::vec3 lightInvDir = -glm::normalize(lightDirection) * 20.0f;
  // Center shadow frustum on camera XZ position for shadows that follow camera
  glm::vec3 frustumCenter = glm::vec3(cameraPosition.x, 0.0f, cameraPosition.z);
  glm::mat4 lightView =
    glm::lookAt(lightInvDir + frustumCenter, frustumCenter, glm::vec3(0, 1, 0));

  return lightProjection * lightView;
}

} // namespace LightingUtil

#endif // LIGHTINGUTIL_H_
