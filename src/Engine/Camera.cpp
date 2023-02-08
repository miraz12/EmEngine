#include "Camera.hpp"

#include <iostream>
#include <ostream>

#include "Window.hpp"

#ifdef EMSCRIPTEN
#define GL_OES_vertex_array_object
#include <GLES3/gl3.h>
#include <emscripten.h>
#else
#include <glad/glad.h>
#endif

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

void Camera::bindProjViewMatrix(unsigned int proj, unsigned int view) {
  if (m_matrixNeedsUpdate) {
    m_viewMatrix = glm::lookAt(m_position, m_position + m_front, m_up);
    m_ProjectionMatrix = glm::perspective(glm::radians(m_fov), m_width / m_height, 0.1f, 1000.0f);
    m_matrixNeedsUpdate = false;
  }
  glUniformMatrix4fv(proj, 1, GL_FALSE, glm::value_ptr(m_ProjectionMatrix));
  glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(m_viewMatrix));
}
