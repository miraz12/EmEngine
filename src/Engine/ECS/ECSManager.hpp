#ifndef ECSMANAGER_H_
#define ECSMANAGER_H_

#include "ComponentPool.hpp"
#include "Systems/System.hpp"
#include <SceneLoader.hpp>
#include <Types/LightTypes.hpp>

#ifndef NDEBUG
class Profiler;
#endif

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

  // Emplace a component in-place (preferred)
  template<typename T, typename... Args>
  T& emplaceComponent(Entity entity, Args&&... args)
  {
    u32 index = getComponentTypeID<T>();
    ensurePool<T>(index);
    T& comp = getPool<T>(index)->emplace(entity, std::forward<Args>(args)...);
    m_entityComponentMasks[entity].set(index);
    return comp;
  }

  // Add a component by move
  template<typename T>
  T& addComponent(Entity entity, T&& component)
  {
    return emplaceComponent<std::decay_t<T>>(entity, std::move(component));
  }

  // Check if the entity has a component of the specified type
  template<typename T>
  bool hasComponent(Entity entity)
  {
    std::size_t idx = getComponentTypeID<T>();
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
    } else {
      // T has not been registered yet, assign it a new ID and return it
      u32 next = m_nextComponentTypeID++;
      m_componentTypeToIndex.insert({ type, next });
    }
    return type;
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
    Signature requiredComponents;
    ((requiredComponents.set(getComponentTypeID<T>()), ...));

    std::vector<Entity> matchingEntities;
    matchingEntities.reserve(m_entities.size() / 2);

    for (Entity entity : m_entities) {
      if ((m_entityComponentMasks[entity] & requiredComponents) ==
          requiredComponents) {
        matchingEntities.push_back(entity);
      }
    }

    return matchingEntities;
  }

  // Get the component from the pool, returns nullptr if not found
  template<typename T>
  T* getComponent(Entity entity)
  {
    if (!hasComponent<T>(entity)) {
      return nullptr;
    }

    std::size_t typeID = getComponentTypeID<T>();

    if (typeID >= m_componentPools.size() || !m_componentPools[typeID]) {
      return nullptr;
    }

    return getPool<T>(typeID)->get(entity);
  }

  // Get a typed pool for direct iteration
  template<typename T>
  ComponentPool<T>* getPool()
  {
    u32 index = getComponentTypeID<T>();
    if (index >= m_componentPools.size() || !m_componentPools[index]) {
      return nullptr;
    }
    return getPool<T>(index);
  }

  // Create point light
  std::shared_ptr<PointLight> setupPointLight(Entity entity,
                                              glm::vec3 color,
                                              float constant,
                                              float linear,
                                              float quadratic,
                                              glm::vec3 pos);
  // Create directional light
  std::shared_ptr<DirectionalLight> setupDirectionalLight(Entity entity,
                                                          glm::vec3 color,
                                                          float intensity,
                                                          glm::vec3 dir);
  void updateDirLight(glm::vec3 color, float intensity, glm::vec3 dir);

  System& getSystem(std::string const& system) { return *m_systems[system]; }
  Entity getPickedEntity() const { return m_pickedEntity; }
  bool getEntitySelected() const { return m_entitySelected; }
  const std::vector<Entity>& getEntities() const { return m_entities; }
  std::string_view getEntityName(Entity entity) const
  {
    auto it = m_entityNames.find(entity);
    if (it != m_entityNames.end())
      return it->second;
    return {};
  }
  bool getSimulatePhysics() const { return m_simulatePhysics; }
  bool getRenderGraphics() const { return m_renderGraphics; }
  i32 getDebugView() const { return m_debugView; }

  // Mutable refs for ImGui widget binding
  bool& refSimulatePhysics() { return m_simulatePhysics; }
  i32& refDebugView() { return m_debugView; }

  void setViewport(u32 width, u32 height);
  void setPickedEntity(Entity entity) { m_pickedEntity = entity; }
  void setEntitySelected(bool sel) { m_entitySelected = sel; }
  void setSimulatePhysics(bool sim) { m_simulatePhysics = sim; }
  void setRenderGraphics(bool sim) { m_renderGraphics = sim; }

  glm::vec3 dDir;

private:
  ECSManager() = default;
  ~ECSManager() = default;

  template<typename T>
  void ensurePool(u32 index)
  {
    if (m_componentPools.size() <= index) {
      m_componentPools.resize(index + 1);
    }
    if (!m_componentPools[index]) {
      m_componentPools[index] = std::make_unique<ComponentPool<T>>();
    }
  }

  template<typename T>
  ComponentPool<T>* getPool(u32 index)
  {
    return static_cast<ComponentPool<T>*>(m_componentPools[index].get());
  }

  // Entities
  std::vector<Entity> m_entities;
  std::map<Entity, std::string> m_entityNames;
  std::vector<System*> m_systemUpdateOrder;
  std::unordered_map<std::string, System*> m_systems;

  // Component pools - one pool per component type
  std::vector<std::unique_ptr<IComponentPool>> m_componentPools;

  // Maps component types to their indices
  std::unordered_map<ComponentType, size_t> m_componentTypeToIndex;

  // Tracks which components each entity has
  std::array<Signature, MAX_ENTITIES> m_entityComponentMasks;

  size_t m_entityCount{ 1 };
  size_t m_nextComponentTypeID{ 0 };
  Entity m_pickedEntity{ 0 };
  bool m_entitySelected{ false };
  Entity m_dirLightEntity{ 0 };
  bool m_simulatePhysics{ false };
  bool m_renderGraphics{ false };
  i32 m_debugView{ 0 };

  // Entity ID reuse system
  std::queue<Entity> m_availableEntityIds;

#ifndef NDEBUG
  Profiler* m_profiler{ nullptr };

public:
  void setProfiler(Profiler* prof) { m_profiler = prof; }
#endif
};
#endif // ECSMANAGER_H_
