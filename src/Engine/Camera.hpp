#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {
public:
  Camera();
  ~Camera();

  void setPosition(glm::vec3 v) {
    m_position = v;
    m_matrixNeedsUpdate = true;
  };

  void setZoom(float zoomAmount) {
    m_zoom = zoomAmount;
    m_matrixNeedsUpdate = true;
  }

  void setFront(glm::vec3 f) {
    m_front = f;
    m_matrixNeedsUpdate = true;
  };
  void setSize(u32 w, u32 h) {
    m_width = w;
    m_height = h;
    m_matrixNeedsUpdate = true;
  }

  void setFOV(float fov) {
    m_fov = fov;
    m_matrixNeedsUpdate = true;
  };

  void setNear(float near) {
    m_near = near;
    m_matrixNeedsUpdate = true;
  };

  void setFar(float far) {
    m_far = far;
    m_matrixNeedsUpdate = true;
  };

  float getFOV() { return m_fov; };
  float getNear() { return m_near; };
  float getFar() { return m_far; };
  float getWidth() { return m_width; };
  float getHeight() { return m_height; };
  glm::vec3 getPosition() { return m_position; };
  glm::vec3 getFront() { return m_front; };
  glm::vec3 getUp() { return m_up; };
  std::tuple<float, float> getSize() { return {m_width, m_height}; };
  std::tuple<glm::vec3, glm::vec3> getRayTo(i32 x, i32 y);
  glm::mat4 &getViewMatrix() { return m_viewMatrix; }
  glm::mat4 &getProjectionMatrix() { return m_ProjectionMatrix; }

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
