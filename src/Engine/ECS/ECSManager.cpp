#include "ECSManager.hpp"
#ifndef NDEBUG
#include "Profiler.hpp"
#endif
#include "Components/GraphicsComponent.hpp"
#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/AudioSourceComponent.hpp"
#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "Objects/GltfObject.hpp"
#include "Systems/AnimationSystem.hpp"
#include "Systems/AudioSystem.hpp"
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
  m_systems["AUDIO"] = &AudioSystem::getInstance();

  // Explicit update order: camera -> particles -> animation -> audio -> position -> graphics -> physics
  // Audio runs after animation (needs camera for listener) but before graphics.
  // Physics runs last so that forces/velocities set by the game layer (C# via C API)
  // between frames are consumed in the same frame's physics step.
  m_systemUpdateOrder = {
    m_systems["CAMERA"],
    m_systems["PARTICLES"],
    m_systems["ANIMATION"],
    m_systems["AUDIO"],
    m_systems["POSITION"],
    m_systems["GRAPHICS"],
    m_systems["PHYSICS"],
  };

  for (auto* system : m_systemUpdateOrder) {
    system->initialize(*this);
  }
}

void
ECSManager::update(float dt)
{
#ifndef NDEBUG
  static constexpr std::string_view kSystemNames[] = {
    "Camera", "Particles", "Animation", "Audio", "Position", "Graphics", "Physics"
  };
#endif

  for (size_t i = 0; i < m_systemUpdateOrder.size(); ++i) {
#ifndef NDEBUG
    // Skip timing Graphics — its internals are covered by render pass sections
    bool profileThis = m_profiler && kSystemNames[i] != "Graphics";
    if (profileThis)
      m_profiler->beginSection(kSystemNames[i], SectionCategory::kSystem);
#endif
    m_systemUpdateOrder[i]->update(dt);
#ifndef NDEBUG
    if (profileThis)
      m_profiler->endSection();
#endif
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
  auto pLight = std::make_shared<PointLight>();
  pLight->position = pos;
  pLight->color = color;
  pLight->constant = constant;
  pLight->linear = linear;
  pLight->quadratic = quadratic;
  emplaceComponent<LightingComponent>(
    entity, pLight, LightingComponent::TYPE::POINT);
  return pLight;
}

std::shared_ptr<DirectionalLight>
ECSManager::setupDirectionalLight(Entity entity,
                                  glm::vec3 color,
                                  float intensity,
                                  glm::vec3 dir)
{
  m_dirLightEntity = entity;
  auto dLight = std::make_shared<DirectionalLight>();
  dLight->direction = dir;
  dLight->color = color;
  dLight->intensity = intensity;
  emplaceComponent<LightingComponent>(
    m_dirLightEntity, dLight, LightingComponent::TYPE::DIRECTIONAL);
  return dLight;
}

void
ECSManager::updateDirLight(glm::vec3 color, float intensity, glm::vec3 dir)
{
  auto* lComp = getComponent<LightingComponent>(m_dirLightEntity);
  if (!lComp)
    return;
  auto* dLight = static_cast<DirectionalLight*>(lComp->light.get());
  dLight->direction = dir;
  dLight->color = color;
  dLight->intensity = intensity;
}

extern "C" void
SetMainCamera(int entity)
{
  CameraSystem::getInstance().setMainCamera(entity);
}

void
ECSManager::setViewport(u32 width, u32 height)
{
  auto cam = CameraSystem::getInstance().getMainCameraComponent();
  if (cam) {
    cam->m_width = width;
    cam->m_height = height;
  }

  static_cast<GraphicsSystem*>(m_systems["GRAPHICS"])
    ->setViewport(width, height);
}

void
ECSManager::reset()
{
  // Clear all entities
  m_entities.clear();
  m_entityNames.clear();

  // Reset all component pools
  for (auto& pool : m_componentPools) {
    if (pool) {
      pool->clear();
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
    return PhysicsSystem::getInstance().entityOnGround(entity);
  }

  void SetRotation(unsigned int entity, float angle)
  {
    auto p = ECSManager::getInstance().getComponent<PositionComponent>(entity);
    if (!p)
      return;
    p->rotation = glm::eulerAngleY(angle);
  }

  void SetVelocity(unsigned int entity, float x, float y, float z)
  {
    auto phy = ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    if (phy && phy->isValid()) {
      PhysicsSystem::getInstance().setLinearVelocity(phy->getBodyID(), x, y, z);
    }
  }

  void SetHorizontalVelocity(unsigned int entity, float x, float z)
  {
    auto phy = ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    if (phy && phy->isValid()) {
      PhysicsSystem::getInstance().setHorizontalVelocity(
        phy->getBodyID(), x, z);
    }
  }

  void GetVelocity(unsigned int entity, float* velocity)
  {
    velocity[0] = 0.0f;
    velocity[1] = 0.0f;
    velocity[2] = 0.0f;
    auto phy = ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    if (phy && phy->isValid()) {
      PhysicsSystem::getInstance().getLinearVelocity(phy->getBodyID(),
                                                     velocity);
    }
  }

  void AddImpulse(unsigned int entity, float x, float y, float z)
  {
    auto phy = ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    if (phy && phy->isValid()) {
      PhysicsSystem::getInstance().addImpulse(phy->getBodyID(), x, y, z);
    }
  }

  void AddForce(unsigned int entity, float x, float y, float z)
  {
    auto phy = ECSManager::getInstance().getComponent<PhysicsComponent>(entity);
    if (phy && phy->isValid()) {
      PhysicsSystem::getInstance().addForce(phy->getBodyID(), x, y, z);
    }
  }

  void AddGraphicsComponent(int entity, const char* model)
  {
    auto gltfObj =
      std::make_shared<GltfObject>("resources/Models/" + std::string(model));
    bool hasAnims = gltfObj->p_numAnimations > 0;
    ECSManager::getInstance().emplaceComponent<GraphicsComponent>(
      entity, std::move(gltfObj));
    if (hasAnims) {
      ECSManager::getInstance().emplaceComponent<AnimationComponent>(entity);
    }
  }

  void AddPositionComponent(int entity,
                            float pos[3],
                            float scale[3],
                            float rot[3])
  {
    auto& posComp =
      ECSManager::getInstance().emplaceComponent<PositionComponent>(entity);
    posComp.position = glm::vec3(pos[0], pos[1], pos[2]);
    posComp.scale = glm::vec3(scale[0], scale[1], scale[2]);
    posComp.rotation =
      glm::vec3(rot[0], rot[1], rot[2]); // TODO is this correct?
  }

  void AddPhysicsComponent(int entity, float mass, int type)
  {
    ECSManager::getInstance().emplaceComponent<PhysicsComponent>(
      entity, entity, mass, CollisionShapeType(type));
  }

  void AddCameraComponent(int entity, bool main, float offset[3])
  {
    auto& cam = ECSManager::getInstance().emplaceComponent<CameraComponent>(
      entity, glm::vec3(offset[0], offset[1], offset[2]));
    CameraSystem::tilt(&cam, -30.0f);
    if (main) {
      CameraSystem::getInstance().setMainCamera(entity);
    }
  }

  void SetCameraOrientation(int entity,
                            float frontX,
                            float frontY,
                            float frontZ,
                            float upX,
                            float upY,
                            float upZ)
  {
    auto* cam = ECSManager::getInstance().getComponent<CameraComponent>(entity);
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
    if (!a)
      return;
    a->isPlaying = false;
  }

  void StartAnimation(unsigned int entity)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    if (!a)
      return;
    a->isPlaying = true;
  }

  void SetAnimationIndex(unsigned int entity, unsigned int idx)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    if (!a)
      return;
    if (a->animationIndex != idx || a->blending) {
      a->animationIndex = idx;
      a->currentTime = 0;
      a->blending = false;
    }
  }

  void CrossfadeAnimation(unsigned int entity,
                          unsigned int targetIndex,
                          float duration)
  {
    auto a = ECSManager::getInstance().getComponent<AnimationComponent>(entity);
    if (!a)
      return;

    // Already playing or blending to this target — nothing to do
    if (a->animationIndex == targetIndex)
      return;

    // Zero/negative duration — hard cut
    if (duration <= 0.0f) {
      a->animationIndex = targetIndex;
      a->currentTime = 0.0f;
      a->blending = false;
      return;
    }

    a->blendFromIndex = a->animationIndex;
    a->blendFromTime = a->currentTime;
    a->animationIndex = targetIndex;
    a->currentTime = 0.0f;
    a->blending = true;
    a->blendWeight = 0.0f;
    a->blendDuration = duration;
    a->blendElapsed = 0.0f;
  }

  bool GetSimulatePhysics()
  {
    return ECSManager::getInstance().getSimulatePhysics();
  }

  // Audio API
  void AddAudioSourceComponent(int entity, const char* clipPath)
  {
    ECSManager::getInstance().emplaceComponent<AudioSourceComponent>(
      entity, std::string(clipPath));
  }

  void Audio_Play(unsigned int entity)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio) {
      audio->play = true;
      audio->stop = false;
    }
  }

  void Audio_Stop(unsigned int entity)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio) {
      audio->stop = true;
      audio->play = false;
    }
  }

  void Audio_SetVolume(unsigned int entity, float volume)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio)
      audio->volume = volume;
  }

  void Audio_SetPitch(unsigned int entity, float pitch)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio)
      audio->pitch = pitch;
  }

  void Audio_SetLoop(unsigned int entity, bool loop)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio)
      audio->loop = loop;
  }

  void Audio_Set3D(unsigned int entity, bool is3D)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio)
      audio->is3D = is3D;
  }

  void Audio_SetMinDistance(unsigned int entity, float dist)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio)
      audio->minDistance = dist;
  }

  void Audio_SetMaxDistance(unsigned int entity, float dist)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    if (audio)
      audio->maxDistance = dist;
  }

  bool Audio_IsPlaying(unsigned int entity)
  {
    auto* audio =
      ECSManager::getInstance().getComponent<AudioSourceComponent>(entity);
    return audio ? audio->isPlaying : false;
  }

  void Audio_SetMasterVolume(float volume)
  {
    AudioSystem::getInstance().setMasterVolume(volume);
  }

  float Audio_GetMasterVolume()
  {
    return AudioSystem::getInstance().getMasterVolume();
  }

  void Audio_LoadClip(const char* path)
  {
    AudioSystem::getInstance().loadAudioClip(path);
  }
};
