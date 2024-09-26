#ifndef CAMERA_H_
#define CAMERA_H_

class Camera {
public:
  Camera() = default;
  ~Camera() = default;

  void setPosition(float x, float y, float z) {
    m_position = glm::vec3(x, y, z);
    m_matrixNeedsUpdate = true;
  };

  void setPosition(glm::vec3 v) {
    m_position = v;
    m_matrixNeedsUpdate = true;
  };

  void setZoom(float zoomAmount) {
    m_zoom = zoomAmount;
    m_matrixNeedsUpdate = true;
  }

  void setFront(float x, float y, float z) {
    m_front = glm::vec3(x, y, z);
    m_matrixNeedsUpdate = true;
  };

  void setFront(glm::vec3 v) {
    m_front = v;
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

  void setNear(float n) {
    m_near = n;
    m_matrixNeedsUpdate = true;
  };

  void setFar(float f) {
    m_far = f;
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

  float *getViewMatrixPtr() { return glm::value_ptr(m_viewMatrix); }
  float *getProjectionMatrixPtr() { return glm::value_ptr(m_ProjectionMatrix); }
  float *getPositionPtr() { return glm::value_ptr(m_position); };
  float *getFrontPtr() { return glm::value_ptr(m_front); };
  float *getUpPtr() { return glm::value_ptr(m_up); };

  void bindProjViewMatrix(u32 proj, u32 view);
  void bindProjMatrix(u32 proj);
  void bindViewMatrix(u32 view);

private:
  void checkDirty();

  glm::mat4 m_viewMatrix{1.0f};
  glm::mat4 m_ProjectionMatrix;
  bool m_matrixNeedsUpdate{true};

  glm::vec3 m_position{0.0f, 0.0f, 3.0f};
  glm::vec3 m_front{0.0f, 0.0f, -1.0f};
  glm::vec3 m_up{0.0f, 1.0f, 0.0f};
  float m_zoom{1.f};
  float m_height{800};
  float m_width{800};
  float m_fov{45.f};
  float m_near{0.1f};
  float m_far{100.f};
};

#endif // CAMERA_H_
