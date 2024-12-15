#ifndef CAMERASYSTEM_H_
#define CAMERASYSTEM_H_

#include "ECS/Components/CameraComponent.hpp"
#include "System.hpp"
#include <glm/glm.hpp>
#include <tuple>

class CameraSystem
  : public System
  , public Singleton<CameraSystem>
{
  friend class Singleton<CameraSystem>;

public:
  void update(float dt) override;

  static void bindProjViewMatrix(std::shared_ptr<CameraComponent> camera,
                                 u32 proj,
                                 u32 view);
  static void bindProjMatrix(std::shared_ptr<CameraComponent> camera, u32 proj);
  static void bindViewMatrix(std::shared_ptr<CameraComponent> camera, u32 view);
  static std::tuple<glm::vec3, glm::vec3>
  getRayTo(std::shared_ptr<CameraComponent> camera, i32 x, i32 y);
  static void tilt(std::shared_ptr<CameraComponent> camera, float angle);

private:
  static void updateMatrices(std::shared_ptr<CameraComponent> camera);
};

#endif // CAMERASYSTEM_H_
