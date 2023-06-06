#include "Camera.hpp"

#include <iostream>
#include <ostream>

Camera::Camera()
    : m_viewMatrix(1.0f), m_matrixNeedsUpdate(true), m_position{0.0f, 0.0f, 3.0f},
      m_front(0.0f, 0.0f, -1.0f), m_up(0.0f, 1.0f, 0.0f), m_zoom(1.0f), m_fov(60.0f), m_height(800),
      m_width(800) {}

Camera::~Camera() {}

glm::mat4 &Camera::getViewMatrix() { return m_viewMatrix; }

void Camera::setZoom(float zoomAmount) {
  m_zoom = zoomAmount;
  m_matrixNeedsUpdate = true;
}

void Camera::bindProjViewMatrix(uint32_t proj, uint32_t view) {
  if (m_matrixNeedsUpdate) {
    m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_fov), m_width / m_height, 0.1f, 1000.0f);
    m_matrixNeedsUpdate = false;
  }
  glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(m_ProjectionMatrix));
  glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
}

std::tuple<glm::vec3, glm::vec3> Camera::getRayTo(int x, int y) {
  glm::mat4 inverseProjectionMatrix = glm::inverse(m_ProjectionMatrix);
  glm::mat4 inverseViewMatrix = glm::inverse(m_viewMatrix);

  // The ray Start and End positions, in Normalized Device Coordinates (Have you read Tutorial 4 ?)
  glm::vec2 ndc(((float)x / (float)m_width - 0.5f) * 2.0f,  // [0,1024] -> [-1,1]
                ((float)y / (float)m_height - 0.5f) * -2.0f // [0, 768] -> [-1,1]
  );
  glm::vec4 clipCoords = inverseProjectionMatrix * glm::vec4(ndc, -1.0, 1.0);
  clipCoords.z = -1.0;
  clipCoords.w = 0.0;

  glm::vec4 worldCoords = inverseViewMatrix * clipCoords;

  glm::vec3 dir = glm::normalize(glm::vec3(worldCoords));

  return {m_position, dir};
}
