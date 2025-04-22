#ifndef ECSMANAGER_H_
#define ECSMANAGER_H_

#include "ComponentPool.hpp"
#include "Components/Component.hpp"
#include "Systems/System.hpp"
#include <SceneLoader.hpp>
#include <Types/LightTypes.hpp>
#include <bitset>
#include <memory>
#include <unordered_map>

using Signature = std::bitset<MAX_COMPONENTS>;

class ECSManager : public Singleton<ECSManager>
{
  friend class Singleton<ECSManager>;

public:
  void initializeSystems();

  // Runs through all systems
  void update(float dt);

  // resets ECS
  void reset();

  // Destroys an entity and recycles its ID
  void destroyEntity(Entity entity);

  void updateRenderingSystems(float dt);

  // creates and returns a new entity
  Entity createEntity(std::string name = "no_name");

  // TODO: Figure out something better than this
  Entity getLastEntity() { return m_entityCount - 1; }

  template<typename T>
  void addComponent(Entity entity, std::shared_ptr<T> component)
  {
    // Get the component type ID
    u32 index = getComponentTypeID<T>();

    // Ensure we have enough component pools
    if (m_componentPools.size() <= index) {
      m_componentPools.resize(index + 1);
    }

    // Create the component pool if it doesn't exist
    if (!m_componentPools[index]) {
      m_componentPools[index] = std::make_shared<ComponentPool<T>>();
    }

    // Add the component to the pool
    std::static_pointer_cast<ComponentPool<T>>(m_componentPools[index])
      ->add(entity, component);

    // Set the component bit for this entity
    m_entityComponentMasks[entity].set(index);
  }

  template<typename... Args>
  void addComponents(Entity entity, std::shared_ptr<Args>... args)
  {
    (addComponent(entity, std::forward<std::shared_ptr<Args>>(args)), ...);
  }

  template<typename T>
  bool hasComponent(Entity entity)
  {
    // Get the component type ID for T
    std::size_t idx = getComponentTypeID<T>();

    // Check if the entity has a component of the specified type
    return m_entityComponentMasks[entity].test(idx);
  }

  template<typename T>
  void removeEntity(Entity entityID);
  template<typename T>
  void removeComponent(Entity& entity, T component);

  template<typename T>
  ComponentType getComponentType()
  {
    ComponentType type = typeid(T);
    if (auto iter = m_componentTypeToIndex.find(type);
        iter != m_componentTypeToIndex.end()) {
      // T has already been registered, return it
      return type;
    } else {
      // T has not been registered yet, assign it a new ID and return it
      i32 next = m_nextComponentTypeID++;
      m_componentTypeToIndex.insert({ type, next });
      return type;
    }
  }
  template<typename T>
  std::size_t getComponentTypeID()
  {
    ComponentType type = getComponentType<T>();
    return m_componentTypeToIndex[type];
  }

  // Create a view of entities with the given component types
  template<typename... T>
  std::vector<Entity> view()
  {
    // Create a bitset representing the required components
    Signature requiredComponents;
    ((requiredComponents.set(getComponentTypeID<T>()), ...));

    // Create a vector to store the matching entities
    std::vector<Entity> matchingEntities;
    matchingEntities.reserve(m_entities.size() / 2); // Reserve reasonable space

    // Iterate only over active entities
    for (Entity entity : m_entities) {
      // Check if the entity has the required components
      if ((m_entityComponentMasks[entity] & requiredComponents) ==
          requiredComponents) {
        // If the entity has the required components, add it to the matching
        // entities vector
        matchingEntities.push_back(entity);
      }
    }

    return matchingEntities;
  }

  template<typename T>
  std::shared_ptr<T> getComponent(Entity entity)
  {
    // Check if the entity has this component
    if (!hasComponent<T>(entity)) {
      return nullptr;
    }

    // Get the component type ID
    std::size_t typeID = getComponentTypeID<T>();

    // Make sure we have a valid component pool
    if (typeID >= m_componentPools.size() || !m_componentPools[typeID]) {
      return nullptr;
    }

    // Get the component from the pool
    return std::static_pointer_cast<ComponentPool<T>>(m_componentPools[typeID])
      ->get(entity);
  }

  // // Create point light
  std::shared_ptr<PointLight> SetupPointLight(Entity en,
                                              glm::vec3 color,
                                              float constant,
                                              float linear,
                                              float quadratic,
                                              glm::vec3 pos);
  // // Create directional light
  std::shared_ptr<DirectionalLight> SetupDirectionalLight(Entity en,
                                                          glm::vec3 color,
                                                          float ambient,
                                                          glm::vec3 dir);
  void updateDirLight(glm::vec3 color, float ambient, glm::vec3 dir);

  std::shared_ptr<Component> getCamera();
  System& getSystem(std::string const& s) { return *m_systems[s]; }
  Entity& getPickedEntity() { return m_pickedEntity; }
  bool& getEntitySelected() { return m_entitySelected; }
  std::vector<Entity>& getEntities() { return m_entities; };
  std::string_view getEntityName(Entity en) { return m_entityNames[en]; };
  bool& getSimulatePhysics() { return m_simulatePhysics; };
  i32& getDebugView() { return m_debugView; };

  void setViewport(u32 w, u32 h);
  void setPickedEntity(Entity en) { m_pickedEntity = en; }
  void setEntitySelected(bool sel) { m_entitySelected = sel; }
  void setSimulatePhysics(bool sim) { m_simulatePhysics = sim; }

  void loadScene(const char* file);
  void saveScene(const char* file);

  glm::vec3 dDir;

private:
  ECSManager() = default;
  ~ECSManager() = default;

  // Entities
  std::vector<Entity> m_entities;
  std::map<Entity, std::string> m_entityNames;
  std::unordered_map<std::string, System*> m_systems;

  // Component pools - one pool per component type
  std::vector<std::shared_ptr<IComponentPool>> m_componentPools;

  // Maps component types to their indices
  std::unordered_map<ComponentType, size_t> m_componentTypeToIndex;

  // Tracks which components each entity has
  std::array<Signature, MAX_ENTITIES> m_entityComponentMasks;

  size_t m_entityCount{ 1 };
  size_t m_nextComponentTypeID{ 0 };
  Entity m_pickedEntity{ 0 };
  bool m_entitySelected{ false };
  Entity m_dirLightEntity;
  bool m_simulatePhysics{ false };
  i32 m_debugView;
};
#endif // LIGHTINGSYSTEM_H_
