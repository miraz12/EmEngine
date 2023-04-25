#include "ECSManager.hpp"

#include "Systems/GraphicsSystem.hpp"

ECSManager::ECSManager() : m_idCounter(1) { initializeSystems(); }

ECSManager::~ECSManager() {
  // Delete all entities and systems
  // for (auto &e : m_entities) {
  //   delete e;
  // }
  // for (auto &s : m_systems) {
  //   delete s.second;
  // }
}

void ECSManager::initializeSystems() {
  // m_systems["LIGHTS"] = new LightingSystem(this, m_camera);
  // m_systems["GRAPHICS"] = new GraphicsSystem(this, m_camera);
}

void ECSManager::update(float dt) {
  // update all systems
  // for (auto &s : m_systems) {
  //   s.second->update(dt);
  // }
}

Entity ECSManager::createEntity() {
  Entity entity = m_entityCount++;
  m_entities.push_back(entity);
  return entity;
}

template <typename T> void ECSManager::addComponent(Entity entity, T component) {
  m_components[Entity] = component;
}

template <typename T> void ECSManager::removeComponent(Entity &entity, T component) {
  // Check if the entity has a component of the given type
  if (m_components.find(entity) != m_components.end()) {
    // Remove the component from the entity's component vector
    m_components[entity][T] = ;
}

int ECSManager::createPlayerEntity(float /* x */, float /* y */, GLFWwindow * /* window */) {
  // Entity &playerEntity = createEntity();
  // playerEntity.setName("Player");
  // playerEntity.makePlayable();
  // // Add components to player
  // GraphicsComponent *graphComp = new GraphicsComponent();
  // MeshShaderProgram *p = new MeshShaderProgram;
  // Mesh *m = new Mesh(*p);
  // m->LoadFlile("resources/Models/gltf/helmet/DamagedHelmet.glb");
  // // m->LoadFlile("resources/Models/gltf/sponza/Sponza.gltf");
  // // m->LoadFlile("/home/shaggy/git/glTF-Sample-Models/2.0/ToyCar/glTF-Binary/ToyCar.glb");
  // // m->LoadFlile("resources/Models/gltf/sponza/Sponza.gltf");
  // graphComp->grapObj = m;

  // addComponent(playerEntity, graphComp);
  // PositionComponent *posComp = new PositionComponent();
  // // posComp->scale = glm::vec3(0.008, 0.008, 0.008);
  // posComp->rotation = 30.0f;
  // addComponent(playerEntity, posComp);
  // return playerEntity.getID();
}

// PointLight *ECSManager::SetupPointLight(glm::vec3 color, float constant, float linear,
//                                         float quadratic, glm::vec3 pos) {
//   Entity &en = createEntity();
//   PointLight *pLight = new PointLight();
//   pLight->position = pos;
//   pLight->color = color;
//   pLight->constant = constant;
//   pLight->linear = linear;
//   pLight->quadratic = quadratic;
//   LightingComponent *lightComp = new LightingComponent(pLight, LightingComponent::TYPE::POINT);
//   addComponent(en, lightComp);
//   return pLight;
// }
// DirectionalLight *ECSManager::SetupDirectionalLight(glm::vec3 color, float ambient, glm::vec3
// dir) {
//   Entity &en = createEntity();
//   DirectionalLight *dLight = new DirectionalLight();
//   dLight->direction = dir;
//   dLight->color = color;
//   dLight->ambientIntensity = ambient;
//   LightingComponent *lightComp =
//       new LightingComponent(dLight, LightingComponent::TYPE::DIRECTIONAL);
//   addComponent(en, lightComp);
//   return dLight;
// }
