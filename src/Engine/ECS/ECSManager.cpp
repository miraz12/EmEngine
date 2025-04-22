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
  Entity newEntity = (m_entityCount++);
  m_entities.push_back(newEntity);
  m_entityNames[newEntity] = name;
  
  // Initialize the component mask for this entity
  m_entityComponentMasks[newEntity].reset();
  
  return m_entities.back();
}

std::shared_ptr<PointLight>
ECSManager::SetupPointLight(Entity en,
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
  addComponent(en,
               std::make_shared<LightingComponent>(
                 pLight, LightingComponent::TYPE::POINT));
  return pLight;
}

std::shared_ptr<DirectionalLight>
ECSManager::SetupDirectionalLight(Entity en,
                                  glm::vec3 color,
                                  float ambient,
                                  glm::vec3 dir)
{
  m_dirLightEntity = en;
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

  for (auto& e : view) {
    std::shared_ptr<CameraComponent> c = getComponent<CameraComponent>(e);

    if (c->mainCamera) {
      return c;
    }
  }
  return nullptr;
}

void
ECSManager::setViewport(u32 w, u32 h)
{
  auto cam =
    static_pointer_cast<CameraComponent>(ECSManager::getInstance().getCamera());
  cam->m_width = w;
  cam->m_height = h;

  static_cast<GraphicsSystem*>(m_systems["GRAPHICS"])->setViewport(w, h);
};

void
ECSManager::loadScene(const char* file)
{
  SceneLoader::getInstance().init(file);
};
void
ECSManager::saveScene(const char* file)
{
  SceneLoader::getInstance().saveScene(file);
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
}

void
ECSManager::destroyEntity(Entity entity)
{
  // Remove the entity from the active entities list
  m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), entity), m_entities.end());
  
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
    posComp->rotation = glm::vec3(rot[0], rot[1], rot[2]);
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

  int CreateEntity(const char* name)
  {
    return ECSManager::getInstance().createEntity(name);
  }

  void PauseAnimation(unsigned int entity)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    a->isPlaying = false;
  }

  void StartAnimation(unsigned int entity)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    a->isPlaying = true;
  }
};
