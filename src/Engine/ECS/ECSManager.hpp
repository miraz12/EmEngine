#ifndef ECSMANAGER_H_
#define ECSMANAGER_H_

#include <API.hpp>
#include <SceneLoader.hpp>
#include <Types/LightTypes.hpp>

#include "Components/Component.hpp"
#include "Components/GraphicsComponent.hpp"
#include "Components/LightingComponent.hpp"
#include "Components/PhysicsComponent.hpp"
#include "Components/PositionComponent.hpp"
#include "Systems/GraphicsSystem.hpp"
#include "Systems/ParticleSystem.hpp"
#include "Systems/PhysicsSystem.hpp"
#include "Systems/PositionSystem.hpp"
#include "Systems/System.hpp"

typedef std::type_index ComponentType;

#define MAX_COMPONENTS 10
#define MAX_ENTITIES 10
typedef std::bitset<MAX_COMPONENTS> Signature;

class ECSManager : public Singleton<ECSManager>, public API::ECS {
  friend class Singleton<ECSManager>;

public:
  void initializeSystems();

  // Runs through all systems
  void update(float dt) override;

  glm::vec3 &getPosition(Entity en) override {
    return getComponent<PositionComponent>(en)->position;
  };
  glm::quat &getRotation(Entity en) override {
    return getComponent<PositionComponent>(en)->rotation;
  };
  glm::vec3 &getScale(Entity en) override {
    return getComponent<PositionComponent>(en)->scale;
  };

  // resets ECS
  void reset();

  void updateRenderingSystems(float dt);

  // creates and returns a new entity
  Entity createEntity(std::string name = "no_name");

  // TODO: Figure out something better than this
  Entity getLastEntity() { return m_entityCount - 1; }

  template <typename T>
  void addComponent(Entity entity, std::shared_ptr<T> component) {
    u32 index = getComponentTypeID<T>();
    m_components[entity][index] = component;
    m_entityComponentMasks[entity] |= (1 << index);
  }

  template <typename... Args>
  void addComponents(Entity entity, std::shared_ptr<Args>... args) {
    (addComponent(entity, std::forward<std::shared_ptr<Args>>(args)), ...);
  }

  template <typename T> bool hasComponent(Entity entity) {
    // Get the component type ID for T
    std::size_t idx = getComponentTypeID<T>();

    // Check if the entity has a component of the specified type

    Signature entMask = m_entityComponentMasks[entity];
    Signature compMask = (1UL << idx);
    Signature resMask = (entMask & compMask);
    if (resMask.all()) {
      return true;
    } else {
      // Entity has no components
      return false;
    }
  }

  template <typename T> void removeEntity(Entity entityID);
  template <typename T> void removeComponent(Entity &entity, T component);

  template <typename T> ComponentType getComponentType() {
    ComponentType type = typeid(T);
    if (auto iter = m_componentTypeToIndex.find(type);
        iter != m_componentTypeToIndex.end()) {
      // T has already been registered, return it
      return type;
    } else {
      // T has not been registered yet, assign it a new ID and return it
      i32 next = m_nextComponentTypeID++;
      m_componentTypeToIndex.insert({type, next});
      return type;
    }
  }
  template <typename T> std::size_t getComponentTypeID() {
    ComponentType type = getComponentType<T>();
    return m_componentTypeToIndex[type];
  }

  // Create a view of entities with the given component types
  template <typename... T> auto view() {
    // Create a bitset representing the required components
    Signature requiredComponents;
    ((requiredComponents.set(getComponentTypeID<T>()), ...));

    // Create a vector to store the matching entities
    std::vector<Entity> matchingEntities;

    // Iterate over all entities
    for (Entity entity = 0; entity < m_entityCount; entity++) {
      // Check if the entity has the required components
      Signature entityMask = m_entityComponentMasks[entity];
      if ((entityMask & requiredComponents) == requiredComponents) {
        // If the entity has the required components, add it to the matching
        // entities vector
        matchingEntities.push_back(entity);
      }
    }

    // Return the vector of matching entities
    return matchingEntities;
  }

  template <typename T> std::shared_ptr<T> getComponent(Entity entity) {
    // Return the component at the specified entity's index in the array
    assert(!hasComponent<T>(entity));
    return std::dynamic_pointer_cast<T>(
        m_components.at(entity).at(getComponentTypeID<T>()));
  }

  // // Create point light
  std::shared_ptr<PointLight> SetupPointLight(Entity en, glm::vec3 color,
                                              float constant, float linear,
                                              float quadratic, glm::vec3 pos);
  // // Create directional light
  std::shared_ptr<DirectionalLight> SetupDirectionalLight(Entity en,
                                                          glm::vec3 color,
                                                          float ambient,
                                                          glm::vec3 dir);
  void updateDirLight(glm::vec3 color, float ambient, glm::vec3 dir);

  Camera &getCamera() override { return m_camera; };
  System &getSystem(std::string s) { return *m_systems[s]; }
  Entity &getPickedEntity() override { return m_pickedEntity; }
  bool &getEntitySelected() { return m_entitySelected; }
  std::vector<Entity> &getEntities() { return m_entities; };
  std::string_view getEntityName(Entity en) { return m_entityNames[en]; };
  bool &getSimulatePhysics() override { return m_simulatePhysics; };
  i32 &getDebugView() override { return m_debugView; };

  void setViewport(u32 w, u32 h) override;
  void setPickedEntity(Entity en) { m_pickedEntity = en; }
  void setEntitySelected(bool sel) { m_entitySelected = sel; }
  void setSimulatePhysics(bool sim) override { m_simulatePhysics = sim; }

  void loadScene(std::string_view file) override;
  void saveScene(std::string_view file) override;

  glm::vec3 dDir;

private:
  ECSManager();
  ~ECSManager();
  // Entities
  std::vector<Entity> m_entities;
  std::map<Entity, std::string> m_entityNames;
  std::unordered_map<std::string, System *> m_systems;

  std::unordered_map<Entity, std::vector<std::shared_ptr<Component>>>
      m_components;
  std::unordered_map<ComponentType, size_t> m_componentTypeToIndex;
  std::unordered_map<Entity, Signature> m_entityComponentMasks;

  size_t m_entityCount{0};
  size_t m_nextComponentTypeID{0};
  Entity m_pickedEntity{0};
  bool m_entitySelected{false};
  Entity m_dirLightEntity;
  bool m_simulatePhysics{false};
  i32 m_debugView;

  Camera m_camera;
};
#endif // LIGHTINGSYSTEM_H_
