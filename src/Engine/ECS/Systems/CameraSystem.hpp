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

  static void updateCameraUBO(CameraComponent* camera);

  static std::tuple<glm::vec3, glm::vec3> getRayTo(CameraComponent* camera,
                                                   i32 x,
                                                   i32 y);
  static void tilt(CameraComponent* camera, float angle);

  void setMainCamera(Entity entity) { m_mainCamera = entity; }
  Entity getMainCamera() const { return m_mainCamera; }

  CameraComponent* getMainCameraComponent();

private:
  static void updateMatrices(CameraComponent* camera);
  Entity m_mainCamera{ 0 };
};

#endif // CAMERASYSTEM_H_
