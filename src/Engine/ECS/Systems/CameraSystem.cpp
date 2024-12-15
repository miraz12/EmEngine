#include "CameraSystem.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"

void
CameraSystem::update(float /* dt */)
{
  std::vector<Entity> view = m_manager->view<CameraComponent>();

  for (auto& e : view) {
    std::shared_ptr<CameraComponent> c =
      m_manager->getComponent<CameraComponent>(e);

    if (std::shared_ptr<PositionComponent> p =
          m_manager->getComponent<PositionComponent>(e);
        p) {
      c->m_position = p->position + c->m_offset;
      updateMatrices(c);
    } else if (c->m_matrixNeedsUpdate) {
      updateMatrices(c);
    }
  }
}

void
CameraSystem::updateMatrices(std::shared_ptr<CameraComponent> camera)
{
  camera->m_viewMatrix = glm::lookAt(
    camera->m_position, camera->m_position + camera->m_front, camera->m_up);

  camera->m_ProjectionMatrix =
    glm::perspective(glm::radians(camera->m_fov),
                     camera->m_width / camera->m_height,
                     camera->m_near,
                     camera->m_far);

  camera->m_matrixNeedsUpdate = false;
}

void
CameraSystem::bindProjViewMatrix(std::shared_ptr<CameraComponent> camera,
                                 u32 proj,
                                 u32 view)
{
  if (camera->m_matrixNeedsUpdate) {
    updateMatrices(camera);
  }
  glUniformMatrix4fv(
    proj, 1, GL_FALSE, glm::value_ptr(camera->m_ProjectionMatrix));
  glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(camera->m_viewMatrix));
}

void
CameraSystem::bindProjMatrix(std::shared_ptr<CameraComponent> camera, u32 proj)
{
  if (camera->m_matrixNeedsUpdate) {
    updateMatrices(camera);
  }
  glUniformMatrix4fv(
    proj, 1, GL_FALSE, glm::value_ptr(camera->m_ProjectionMatrix));
}

void
CameraSystem::bindViewMatrix(std::shared_ptr<CameraComponent> camera, u32 view)
{
  if (camera->m_matrixNeedsUpdate) {
    updateMatrices(camera);
  }
  glUniformMatrix4fv(view, 1, GL_FALSE, glm::value_ptr(camera->m_viewMatrix));
}

std::tuple<glm::vec3, glm::vec3>
CameraSystem::getRayTo(std::shared_ptr<CameraComponent> camera, i32 x, i32 y)
{
  glm::mat4 inverseProjectionMatrix = glm::inverse(camera->m_ProjectionMatrix);
  glm::mat4 inverseViewMatrix = glm::inverse(camera->m_viewMatrix);

  glm::vec2 ndc(((float)x / camera->m_width - 0.5f) * 2.0f,
                ((float)y / camera->m_height - 0.5f) * -2.0f);

  glm::vec4 clipCoords = inverseProjectionMatrix * glm::vec4(ndc, -1.0, 1.0);
  clipCoords.z = -1.0;
  clipCoords.w = 0.0;

  glm::vec4 worldCoords = inverseViewMatrix * clipCoords;
  glm::vec3 dir = glm::normalize(glm::vec3(worldCoords));

  return { camera->m_position, dir };
}

void
CameraSystem::tilt(std::shared_ptr<CameraComponent> camera, float angle)
{
  glm::vec3 right = glm::normalize(glm::cross(camera->m_front, camera->m_up));

  glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), right);

  camera->m_front =
    glm::normalize(glm::vec3(rotation * glm::vec4(camera->m_front, 0.0f)));
  camera->m_up =
    glm::normalize(glm::vec3(rotation * glm::vec4(camera->m_up, 0.0f)));

  camera->m_matrixNeedsUpdate = true;
}
