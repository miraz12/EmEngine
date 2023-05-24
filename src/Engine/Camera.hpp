#ifndef CAMERA_H_
#define CAMERA_H_

#include <glm/ext.hpp>
#include <glm/glm.hpp>


class Camera {
public:
  Camera();
  ~Camera();

  glm::mat4 &getViewMatrix();

  void setPosition(glm::vec3 v) {
    m_position = v;
    m_matrixNeedsUpdate = true;
  };
  void setZoom(float zoomAmount);
  void setFront(glm::vec3 f) {
    m_front = f;
    m_matrixNeedsUpdate = true;
  };
  void setSize(unsigned int w, unsigned int h) {
    m_width = w;
    m_height = h;
    m_matrixNeedsUpdate = true;
  }

  glm::vec3 getPosition() { return m_position; };
  glm::vec3 getFront() { return m_front; };
  glm::vec3 getUp() { return m_up; };

  void bindProjViewMatrix(unsigned int proj, unsigned int view);

private:
  glm::mat4 m_viewMatrix;
  glm::mat4 m_ProjectionMatrix;
  bool m_matrixNeedsUpdate;

  glm::vec3 m_position;
  glm::vec3 m_front;
  glm::vec3 m_up;
  float m_zoom;
  float m_fov;
  float m_height;
  float m_width;
};

#endif // CAMERA_H_
