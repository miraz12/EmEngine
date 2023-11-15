#include "Camera.hpp"

Camera::Camera()
    : m_viewMatrix(1.0f),
      m_matrixNeedsUpdate(true), m_position{0.0f, 0.0f, 3.0f},
      m_front(0.0f, 0.0f, -1.0f), m_up(0.0f, 1.0f, 0.0f), m_zoom(1.0f),
      m_height(800), m_width(800), m_fov(45.0f), m_near(0.1f), m_far(100.f) {}

Camera::~Camera() {}


void Camera::checkDirty() {
  if (m_matrixNeedsUpdate) {
    m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_fov),
                                          m_width / m_height, m_near, m_far);
    m_matrixNeedsUpdate = false;
  }
}

void Camera::bindProjViewMatrix(u32 proj, u32 view) {
  checkDirty();
  glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(m_ProjectionMatrix));
  glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
}

void Camera::bindProjMatrix(u32 proj) {
  checkDirty();
  glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(m_ProjectionMatrix));
}
void Camera::bindViewMatrix(u32 view) {
  checkDirty();
  glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
}

std::tuple<glm::vec3, glm::vec3> Camera::getRayTo(i32 x, i32 y) {
  glm::mat4 inverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);
  glm::mat4 inverseViewMatrix = glm::inverse(m_viewMatrix);

  // The ray Start and End positions, in Normalized Device Coordinates (Have you
  // read Tutorial 4 ?)
  glm::vec2 ndc(
      ((float)x / (float)m_width - 0.5f) * 2.0f,  // [0,1024] -> [-1,1]
      ((float)y / (float)m_height - 0.5f) * -2.0f // [0, 768] -> [-1,1]
  );
  glm::vec4 clipCoords = inverseProjectionMatrix * glm::vec4(ndc, -1.0, 1.0);
  clipCoords.z = -1.0;
  clipCoords.w = 0.0;

  glm::vec4 worldCoords = inverseViewMatrix * clipCoords;

  glm::vec3 dir = glm::normalize(glm::vec3(worldCoords));

  return {m_position, dir};
}
