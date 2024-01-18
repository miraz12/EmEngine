#ifndef API_HPP_
#define API_HPP_

typedef std::size_t Entity;

extern "C" {
namespace API {
struct Camera {
  virtual float getWidth() = 0;
  virtual float getHeight() = 0;
  virtual glm::mat4 &getViewMatrix() = 0;
  virtual glm::mat4 &getProjectionMatrix() = 0;
  virtual float getFOV() = 0;
  virtual float getNear() = 0;
  virtual float getFar() = 0;
  virtual glm::vec3 getPosition() = 0;
  virtual glm::vec3 getFront() = 0;
  virtual glm::vec3 getUp() = 0;

  virtual void setPosition(glm::vec3 v) = 0;
  virtual void setFOV(float fov) = 0;
  virtual void setNear(float near) = 0;
  virtual void setFar(float far) = 0;
  virtual void setFront(glm::vec3 f) = 0;
};

struct ECS {
  virtual void update(float dt) = 0;
  virtual void loadScene(std::string_view file) = 0;
  virtual void saveScene(std::string_view file) = 0;

  virtual void setViewport(u32 w, u32 h) = 0;
  virtual void setSimulatePhysics(bool sim) = 0;

  virtual Entity &getPickedEntity() = 0;
  virtual glm::vec3 &getPosition(Entity en) = 0;
  virtual glm::quat &getRotation(Entity en) = 0;
  virtual glm::vec3 &getScale(Entity en) = 0;
  virtual Camera &getCamera() = 0;
  virtual bool &getSimulatePhysics() = 0;
  virtual i32 &getDebugView() = 0;
  static ECS &getECS();
};
} // namespace API
};

#endif // API_HPP_
