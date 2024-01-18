#ifndef CAMERA_H_
#define CAMERA_H_

#include <API.hpp>

class Camera : public API::Camera {
public:
  Camera();
  ~Camera();

  void setPosition(glm::vec3 v) override {
    m_position = v;
    m_matrixNeedsUpdate = true;
  };

  void setZoom(float zoomAmount) {
    m_zoom = zoomAmount;
    m_matrixNeedsUpdate = true;
  }

  void setFront(glm::vec3 f) override {
    m_front = f;
    m_matrixNeedsUpdate = true;
  };

  void setSize(u32 w, u32 h) {
    m_width = w;
    m_height = h;
    m_matrixNeedsUpdate = true;
  }

  void setFOV(float fov) override {
    m_fov = fov;
    m_matrixNeedsUpdate = true;
  };

  void setNear(float n) override {
    m_near = n;
    m_matrixNeedsUpdate = true;
  };

  void setFar(float f) override {
    m_far = f;
    m_matrixNeedsUpdate = true;
  };

  float getFOV() override { return m_fov; };
  float getNear() override { return m_near; };
  float getFar() override { return m_far; };
  float getWidth() override { return m_width; };
  float getHeight() override { return m_height; };
  glm::vec3 getPosition() override { return m_position; };
  glm::vec3 getFront() override { return m_front; };
  glm::vec3 getUp() override { return m_up; };
  std::tuple<float, float> getSize() { return {m_width, m_height}; };
  std::tuple<glm::vec3, glm::vec3> getRayTo(i32 x, i32 y);
  glm::mat4 &getViewMatrix() override { return m_viewMatrix; }
  glm::mat4 &getProjectionMatrix() override { return m_ProjectionMatrix; }

  void bindProjViewMatrix(u32 proj, u32 view);
  void bindProjMatrix(u32 proj);
  void bindViewMatrix(u32 view);

private:
  void checkDirty();

  glm::mat4 m_viewMatrix;
  glm::mat4 m_ProjectionMatrix;
  bool m_matrixNeedsUpdate;

  glm::vec3 m_position;
  glm::vec3 m_front;
  glm::vec3 m_up;
  float m_zoom;
  float m_height;
  float m_width;
  float m_fov;
  float m_near;
  float m_far;
};

#endif // CAMERA_H_
