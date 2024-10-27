#include "PositionSystem.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include <ECS/ECSManager.hpp>

void PositionSystem::update(float /* dt */) {
  std::vector<Entity> view = m_manager->view<PositionComponent>();

  for (auto e : view) {
    std::shared_ptr<PositionComponent> p =
        m_manager->getComponent<PositionComponent>(e);

    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), p->position);
    glm::mat4 rotationMatrix = glm::mat4_cast(p->rotation);
    glm::mat4 scaleMatrix = glm::scale(glm::mat4(1.0f), p->scale);

    p->model = translationMatrix * rotationMatrix * scaleMatrix;
  }
}
