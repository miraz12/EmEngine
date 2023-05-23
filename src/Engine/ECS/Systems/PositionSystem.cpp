#include "PositionSystem.hpp"
#include <ECS/ECSManager.hpp>

PositionSystem::PositionSystem(ECSManager *ECSManager, Camera &cam) : System(ECSManager) {}

void PositionSystem::update(float dt) {

  std::vector<Entity> view = m_manager->view<PositionComponent>();

  for (auto e : view) {
    PositionComponent *p = m_manager->getComponent<PositionComponent>(e);
    glm::mat4 tempMatrix = glm::scale(glm::identity<glm::mat4>(), p->scale);
    tempMatrix = glm::rotate(tempMatrix, p->rotation, glm::vec3(-1.0f, 0.0f, 0.0f));
    tempMatrix = glm::translate(tempMatrix, p->position);
    p->model = tempMatrix;
  }
}
