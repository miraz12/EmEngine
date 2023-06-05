#include "ECSManager.hpp"
#include "ECS/Systems/System.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/PhysicsSystem.hpp"
#include <ECS/Systems/PositionSystem.hpp>
#include <Types/LightTypes.hpp>
#include <memory>

ECSManager::ECSManager() { initializeSystems(); }

ECSManager::~ECSManager() {}

void ECSManager::initializeSystems() {
  m_systems["PHYSICS"] = &PhysicsSystem::getInstance();
  m_systems["POSITION"] = &PositionSystem::getInstance();
  m_systems["GRAPHICS"] = &GraphicsSystem::getInstance();
  for (auto &s : m_systems) {
    s.second->initialize(*this);
  }
}

void ECSManager::update(float dt) {
  // update all systems
  for (auto &s : m_systems) {
    s.second->update(dt);
  }
}

Entity ECSManager::createEntity() {
  Entity newEntity = m_entityCount++;
  m_components[newEntity] = std::vector<std::shared_ptr<Component>>(MAX_COMPONENTS, nullptr);
  m_entities.push_back(newEntity);
  return m_entities.back();
}

std::shared_ptr<PointLight> ECSManager::SetupPointLight(glm::vec3 color, float constant,
                                                        float linear, float quadratic,
                                                        glm::vec3 pos) {
  Entity en = createEntity();
  std::shared_ptr<PointLight> pLight = std::make_shared<PointLight>();
  pLight->position = pos;
  pLight->color = color;
  pLight->constant = constant;
  pLight->linear = linear;
  pLight->quadratic = quadratic;
  addComponent(en, std::make_shared<LightingComponent>(pLight, LightingComponent::TYPE::POINT));
  return pLight;
}

std::shared_ptr<DirectionalLight> ECSManager::SetupDirectionalLight(glm::vec3 color, float ambient,
                                                                    glm::vec3 dir) {
  Entity en = createEntity();
  std::shared_ptr<DirectionalLight> dLight = std::make_shared<DirectionalLight>();
  dLight->direction = dir;
  dLight->color = color;
  dLight->ambientIntensity = ambient;
  addComponent(en,
               std::make_shared<LightingComponent>(dLight, LightingComponent::TYPE::DIRECTIONAL));
  return dLight;
}

void ECSManager::setViewport(uint32_t w, uint32_t h) {
  m_camera.setSize(w, h);
  static_cast<GraphicsSystem *>(m_systems["GRAPHICS"])->setViewport(w, h);
};
