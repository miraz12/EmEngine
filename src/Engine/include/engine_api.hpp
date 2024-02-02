#ifndef ENGINE_API_HPP_
#define ENGINE_API_HPP_

extern "C" {
typedef unsigned int Entity;
enum KEY {
  Escape,
  W,
  A,
  S,
  D,
  F,
  O,
  Space,
  ArrowUp,
  ArrowDown,
  ArrowRight,
  ArrowLeft,
  Mouse1
};

namespace API {
// struct Core {
//   virtual bool initialize() = 0;
//   virtual void update(float dt) = 0;
//   virtual bool open() = 0;

//   static Core &getCore();
// };

struct InputManager {
  virtual bool getKey(KEY k) = 0;

  static InputManager &getInputManager();
};

struct Camera {
  virtual float getWidth() = 0;
  virtual float getHeight() = 0;
  virtual float *getViewMatrixPtr() = 0;
  virtual float *getProjectionMatrixPtr() = 0;
  virtual float getFOV() = 0;
  virtual float getNear() = 0;
  virtual float getFar() = 0;
  virtual float *getPositionPtr() = 0;
  virtual float *getFrontPtr() = 0;
  virtual float *getUpPtr() = 0;

  virtual void setPosition(float x, float y, float z) = 0;
  virtual void setFOV(float fov) = 0;
  virtual void setNear(float near) = 0;
  virtual void setFar(float far) = 0;
  virtual void setFront(float x, float y, float z) = 0;
};

struct ECS {
  virtual void update(float dt) = 0;
  virtual void loadScene(const char *file) = 0;
  virtual void saveScene(const char *file) = 0;

  virtual void setViewport(unsigned int w, unsigned int h) = 0;
  virtual void setSimulatePhysics(bool sim) = 0;

  virtual Entity &getPickedEntity() = 0;
  virtual float *getPositionPtr(Entity en) = 0;
  virtual float *getRotationPtr(Entity en) = 0;
  virtual float *getScalePtr(Entity en) = 0;
  virtual Camera &getCamera() = 0;
  virtual bool &getSimulatePhysics() = 0;
  virtual int &getDebugView() = 0;
  static ECS &getECS();
};
} // namespace API

// Wrapper function for ECS::loadScene
void ECS_loadScene(const char *file);
};

#endif // ENGINE_API_HPP_
