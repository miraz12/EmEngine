#include "ECSManager.hpp"
#include "ECS/Systems/System.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/PhysicsSystem.hpp"

ECSManager::ECSManager() { initializeSystems(); }

ECSManager::~ECSManager() {}

void ECSManager::initializeSystems() {
  m_systems["GRAPHICS"] = new GraphicsSystem(this, m_camera);
  m_systems["PHYSICS"] = new PhysicsSystem(this, m_camera);
}

void ECSManager::update(float dt) {
  // update all systems
  for (auto &s : m_systems) {
    s.second->update(dt);
  }
}

Entity ECSManager::createEntity() {
  Entity newEntity = m_entityCount++;
  m_components[newEntity] = std::vector<Component *>(MAX_COMPONENTS, nullptr);
  m_entities.push_back(newEntity);
  return m_entities.back();
}

PointLight *ECSManager::SetupPointLight(glm::vec3 color, float constant, float linear,
                                        float quadratic, glm::vec3 pos) {
  Entity en = createEntity();
  PointLight *pLight = new PointLight();
  pLight->position = pos;
  pLight->color = color;
  pLight->constant = constant;
  pLight->linear = linear;
  pLight->quadratic = quadratic;
  LightingComponent *lightComp = new LightingComponent(pLight, LightingComponent::TYPE::POINT);
  addComponent(en, lightComp);
  return pLight;
}

DirectionalLight *ECSManager::SetupDirectionalLight(glm::vec3 color, float ambient, glm::vec3 dir) {
  Entity en = createEntity();
  DirectionalLight *dLight = new DirectionalLight();
  dLight->direction = dir;
  dLight->color = color;
  dLight->ambientIntensity = ambient;
  LightingComponent *lightComp =
      new LightingComponent(dLight, LightingComponent::TYPE::DIRECTIONAL);
  addComponent(en, lightComp);
  return dLight;
}

void ECSManager::setViewport(unsigned int w, unsigned int h) {
  m_camera.setSize(w, h);
  static_cast<GraphicsSystem *>(m_systems["GRAPHICS"])->setViewport(w, h);
};
