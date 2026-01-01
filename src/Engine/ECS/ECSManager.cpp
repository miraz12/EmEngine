#include "ECSManager.hpp"
#include "Components/GraphicsComponent.hpp"
#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "Objects/GltfObject.hpp"
#include "Systems/AnimationSystem.hpp"
#include "Systems/CameraSystem.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/ParticleSystem.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/PositionSystem.hpp"
#include <algorithm>
#include <utility>

void
ECSManager::initializeSystems()
{
  m_systems["PHYSICS"] = &PhysicsSystem::getInstance();
  m_systems["POSITION"] = &PositionSystem::getInstance();
  m_systems["PARTICLES"] = &ParticleSystem::getInstance();
  m_systems["ANIMATION"] = &AnimationSystem::getInstance();
  m_systems["GRAPHICS"] = &GraphicsSystem::getInstance();
  m_systems["CAMERA"] = &CameraSystem::getInstance();
  for (const auto& [name, system] : m_systems) {
    system->initialize(*this);
  }
}

void
ECSManager::update(float dt)
{
  // update all systems
  for (const auto& [name, system] : m_systems) {
    system->update(dt);
  }
}

Entity
ECSManager::createEntity(std::string name)
{
  Entity newEntity;

  // First try to reuse an available entity ID
  if (!m_availableEntityIds.empty()) {
    newEntity = m_availableEntityIds.front();
    m_availableEntityIds.pop();
  } else {
    // Check if we've hit the MAX_ENTITIES limit
    if (m_entityCount >= MAX_ENTITIES) {
      // Entity creation failed - return invalid entity ID
      return 0;
    }

    // Create new entity ID
    newEntity = (m_entityCount++);
  }

  m_entities.push_back(newEntity);
  m_entityNames[newEntity] = std::move(name);

  // Initialize the component mask for this entity
  m_entityComponentMasks[newEntity].reset();

  return newEntity;
}

std::shared_ptr<PointLight>
ECSManager::setupPointLight(Entity entity,
                            glm::vec3 color,
                            float constant,
                            float linear,
                            float quadratic,
                            glm::vec3 pos)
{
  std::shared_ptr<PointLight> pLight = std::make_shared<PointLight>();
  pLight->position = pos;
  pLight->color = color;
  pLight->constant = constant;
  pLight->linear = linear;
  pLight->quadratic = quadratic;
  addComponent(entity,
               std::make_shared<LightingComponent>(
                 pLight, LightingComponent::TYPE::POINT));
  return pLight;
}

std::shared_ptr<DirectionalLight>
ECSManager::setupDirectionalLight(Entity entity,
                                  glm::vec3 color,
                                  float ambient,
                                  glm::vec3 dir)
{
  m_dirLightEntity = entity;
  std::shared_ptr<DirectionalLight> dLight =
    std::make_shared<DirectionalLight>();
  dLight->direction = dir;
  dLight->color = color;
  dLight->ambientIntensity = ambient;
  addComponent(m_dirLightEntity,
               std::make_shared<LightingComponent>(
                 dLight, LightingComponent::TYPE::DIRECTIONAL));
  return dLight;
}

void
ECSManager::updateDirLight(glm::vec3 color, float ambient, glm::vec3 dir)
{

  std::shared_ptr<LightingComponent> lComp =
    getComponent<LightingComponent>(m_dirLightEntity);
  auto* dLight = static_cast<DirectionalLight*>(&lComp->getBaseLight());
  dLight->direction = dir;
  dLight->color = color;
  dLight->ambientIntensity = ambient;
}

std::shared_ptr<Component>
ECSManager::getCamera()
{
  std::vector<Entity> view = this->view<CameraComponent>();

  for (auto& entity : view) {
    std::shared_ptr<CameraComponent> camera =
      getComponent<CameraComponent>(entity);

    if (camera->mainCamera) {
      return camera;
    }
  }
  return nullptr;
}

extern "C" void
SetMainCamera(int entity)
{
  ECSManager& ecsManager = ECSManager::getInstance();
  std::vector<Entity> view = ecsManager.view<CameraComponent>();

  for (auto& e : view) {
    std::shared_ptr<CameraComponent> cam =
      ecsManager.getComponent<CameraComponent>(e);
    if (cam) {
      cam->mainCamera = false;
    }
  }

  std::shared_ptr<CameraComponent> targetCam =
    ecsManager.getComponent<CameraComponent>(entity);
  if (targetCam) {
    targetCam->mainCamera = true;
  }
}

void
ECSManager::setViewport(u32 width, u32 height)
{
  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());
  cam->m_width = width;
  cam->m_height = height;

  static_cast<GraphicsSystem*>(m_systems["GRAPHICS"])
    ->setViewport(width, height);
};

void
ECSManager::reset()
{
  // Clear all entities
  m_entities.clear();
  m_entityNames.clear();

  // Reset all component pools
  for (auto& pool : m_componentPools) {
    if (pool) {
      for (Entity entity = 0; entity < MAX_ENTITIES; entity++) {
        pool->entityDestroyed(entity);
      }
    }
  }

  // Reset all component masks
  for (auto& mask : m_entityComponentMasks) {
    mask.reset();
  }

  // Reset entity counter
  m_entityCount = 1;

  // Clear entity ID reuse queue
  while (!m_availableEntityIds.empty()) {
    m_availableEntityIds.pop();
  }
}

void
ECSManager::destroyEntity(Entity entity)
{
  // Validate entity exists
  if (entity == 0 || entity >= MAX_ENTITIES) {
    return; // Invalid entity ID
  }

  // Check if entity actually exists in our active list
  auto entityIt = std::ranges::find(m_entities, entity);
  if (entityIt == m_entities.end()) {
    return; // Entity doesn't exist
  }

  // Remove the entity from the active entities list
  m_entities.erase(entityIt);

  // Clear all components for this entity
  m_entityComponentMasks[entity].reset();

  // Notify all component pools that the entity was destroyed
  for (auto& pool : m_componentPools) {
    if (pool) {
      pool->entityDestroyed(entity);
    }
  }

  // Remove the entity name
  m_entityNames.erase(entity);

  // Add the entity ID back to the reuse pool
  m_availableEntityIds.push(entity);
}

// Api stuff
extern "C"
{
  bool EntityOnGround(unsigned int entity)
  {
    return dynamic_cast<PhysicsSystem*>(
             &ECSManager::getInstance().getSystem("PHYSICS"))
      ->EntityOnGround(entity);
  }

  void SetRotation(unsigned int entity, float angle)
  {
    auto p = ECSManager::getInstance().getComponent<PositionComponent>(entity);
    p->rotation = glm::eulerAngleY(angle);
  }

  void SetVelocity(unsigned int entity, float x, float y, float z)
  {
    std::shared_ptr<PhysicsComponent> phy =
      ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    btVector3 vel = phy->body->getLinearVelocity();
    // Use y parameter if needed, otherwise keep existing Y velocity
    phy->body->setLinearVelocity(btVector3(x, y != 0.0f ? y : vel.getY(), z));
  }

  void SetHorizontalVelocity(unsigned int entity, float x, float z)
  {
    std::shared_ptr<PhysicsComponent> phy =
      ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    btVector3 vel = phy->body->getLinearVelocity();
    // Only change horizontal velocity, preserve Y velocity (gravity/jumping)
    phy->body->setLinearVelocity(btVector3(x, vel.getY(), z));
  }

  void GetVelocity(unsigned int entity, float* velocity)
  {
    std::shared_ptr<PhysicsComponent> phy =
      ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    btVector3 vel = phy->body->getLinearVelocity();
    velocity[0] = vel.getX();
    velocity[1] = vel.getY();
    velocity[2] = vel.getZ();
  }

  void AddImpulse(unsigned int entity, float x, float y, float z)
  {
    std::shared_ptr<PhysicsComponent> phy =
      ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    phy->body->applyCentralImpulse(btVector3(x, y, z));
  }

  void AddForce(unsigned int entity, float x, float y, float z)
  {
    std::shared_ptr<PhysicsComponent> phy =
      ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    phy->body->applyCentralForce(btVector3(x, y, z));
  }

  void AddGraphicsComponent(int entity, const char* model)
  {
    std::shared_ptr<GraphicsComponent> graphComp;
    graphComp = std::make_shared<GraphicsComponent>(
      std::make_shared<GltfObject>("resources/Models/" + std::string(model)));
    if (graphComp->m_grapObj->p_numAnimations > 0) {
      ECSManager::getInstance().addComponents(
        entity, std::make_shared<AnimationComponent>());
    }
    ECSManager::getInstance().addComponents(entity, graphComp);
  }

  void AddPositionComponent(int entity,
                            float pos[3],
                            float scale[3],
                            float rot[3])
  {
    std::shared_ptr<PositionComponent> posComp =
      std::make_shared<PositionComponent>();
    posComp->position = glm::vec3(pos[0], pos[1], pos[2]);
    posComp->scale = glm::vec3(scale[0], scale[1], scale[2]);
    posComp->rotation =
      glm::vec3(rot[0], rot[1], rot[2]); // TODO is this correct?
    ECSManager::getInstance().addComponents(entity, posComp);
  }

  void AddPhysicsComponent(int entity, float mass, int type)
  {
    ECSManager::getInstance().addComponents(
      entity,
      std::make_shared<PhysicsComponent>(
        entity, mass, CollisionShapeType(type)));
  }

  void AddCameraComponent(int entity, bool main, float offset[3])
  {
    std::shared_ptr<CameraComponent> c = std::make_shared<CameraComponent>(
      main, glm::vec3(offset[0], offset[1], offset[2]));
    CameraSystem::tilt(c, -30.0f);
    ECSManager::getInstance().addComponents(entity, c);
  }

  void SetCameraOrientation(int entity,
                            float frontX,
                            float frontY,
                            float frontZ,
                            float upX,
                            float upY,
                            float upZ)
  {
    std::shared_ptr<CameraComponent> cam =
      ECSManager::getInstance().getComponent<CameraComponent>(entity);
    if (cam) {
      cam->m_front = glm::vec3(frontX, frontY, frontZ);
      cam->m_up = glm::vec3(upX, upY, upZ);
      cam->m_matrixNeedsUpdate = true;
    }
  }

  int CreateEntity(const char* name)
  {
    return ECSManager::getInstance().createEntity(name);
  }

  void pauseAnimation(unsigned int entity)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    a->isPlaying = false;
  }

  void StartAnimation(unsigned int entity)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    a->isPlaying = true;
  }

  void SetAnimationIndex(unsigned int entity, unsigned int idx)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    if (a->animationIndex != idx) {
      a->animationIndex = idx;
      a->currentTime = 0;
    }
  }

  bool GetSimulatePhysics()
  {
    return ECSManager::getInstance().getSimulatePhysics();
  }
};
