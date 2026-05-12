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

  /// Update CameraUBO with current camera matrices and position.
  /// Call this once per frame before rendering passes that use CameraData UBO.
  static void updateCameraUBO(std::shared_ptr<CameraComponent> camera);

  static std::tuple<glm::vec3, glm::vec3>
  getRayTo(std::shared_ptr<CameraComponent> camera, i32 x, i32 y);
  static void tilt(std::shared_ptr<CameraComponent> camera, float angle);

  void setMainCamera(Entity entity) { m_mainCamera = entity; }
  Entity getMainCamera() const { return m_mainCamera; }

  /// Get the main camera component (nullptr if none set).
  std::shared_ptr<CameraComponent> getMainCameraComponent();

private:
  static void updateMatrices(std::shared_ptr<CameraComponent> camera);
  Entity m_mainCamera{ 0 };
};

#endif // CAMERASYSTEM_H_
