#ifndef CAMERACOMPONENT_H_
#define CAMERACOMPONENT_H_

#include "Component.hpp"

class CameraComponent : public Component
{
public:
  CameraComponent(bool isMainCam, glm::vec3 offset)
    : mainCamera(isMainCam)
    , m_offset(offset)
  {
  }
  CameraComponent(bool isMainCam,
                  float fov,
                  float width,
                  float height,
                  float near,
                  float far)
    : mainCamera(isMainCam)
    , m_fov(fov)
    , m_width(width)
    , m_height(height)
    , m_near(near)
    , m_far(far)
  {
  }

  bool mainCamera{ false };
  glm::mat4 m_viewMatrix{ 1.0f };
  glm::mat4 m_ProjectionMatrix{ 1.0f };
  glm::vec3 m_position{ 0.0f };
  glm::vec3 m_offset{ 0.0f };
  glm::vec3 m_front{ 0.0f, 0.0f, -1.0f };
  glm::vec3 m_up{ 0.0f, 1.0f, 0.0f };
  float m_fov{ 45.0f };
  float m_width{ 800.0f };
  float m_height{ 600.0f };
  float m_near{ 0.1f };
  float m_far{ 100.0f };
  bool m_matrixNeedsUpdate{ true };
};

#endif // CAMERACOMPONENT_H_
