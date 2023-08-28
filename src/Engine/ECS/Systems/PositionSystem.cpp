#include "PositionSystem.hpp"
#include <ECS/ECSManager.hpp>

void PositionSystem::update(float /* dt */) {
  std::vector<Entity> view = m_manager->view<PositionComponent>();

  for (auto e : view) {
    std::shared_ptr<PositionComponent> p =
        m_manager->getComponent<PositionComponent>(e);
    glm::mat4 tempMatrix = glm::scale(glm::identity<glm::mat4>(), p->scale);
    tempMatrix = glm::translate(tempMatrix, p->position);
    tempMatrix *= glm::mat4_cast(p->rotation);
    p->model = tempMatrix;
  }
}
