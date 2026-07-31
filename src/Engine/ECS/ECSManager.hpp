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

  // Monotonic counter shared by all component types. Assigned once per type
  // on first use, in registration order.
  static std::size_t nextComponentTypeID()
  {
    static std::atomic<std::size_t> counter{ 0 };
    return counter++;
  }

  template<typename T>
  static std::size_t getComponentTypeID()
  {
    static const std::size_t id = nextComponentTypeID();
    assert(id < MAX_COMPONENTS && "Too many component types");
    return id;
  }

  // Create a view of entities with the given component types
  template<typename First, typename... Rest>
  std::vector<Entity> view()
  {
    std::vector<Entity> matching;
    auto* pool = getPool<First>();
    if (!pool) {
      return matching;
    }

    Signature required;
    required.set(getComponentTypeID<First>());
    (required.set(getComponentTypeID<Rest>()), ...);

    matching.reserve(pool->size());
    for (std::size_t i = 0; i < pool->size(); ++i) {
      Entity e = pool->entityAt(i);
      if ((m_entityComponentMasks[e] & required) == required) {
        matching.push_back(e);
      }
    }
    return matching;
  }

  // Execute function for entities with given components, driving from the
  // first type's pool — put the rarest component first.
  template<typename First, typename... Rest, typename Fn>
  void each(Fn&& func)
  {
    auto* firstPool = getPool<First>();
    if (!firstPool) {
      return;
    }

    if constexpr (sizeof...(Rest) == 0) {
      for (std::size_t i = 0; i < firstPool->size(); ++i) {
        func(firstPool->entityAt(i), (*firstPool)[i]);
      }
    } else {
      auto restPools = std::make_tuple(getPool<Rest>()...);
      if ((... || (std::get<ComponentPool<Rest>*>(restPools) == nullptr))) {
        return;
      }

      for (std::size_t i = 0; i < firstPool->size(); ++i) {
        Entity e = firstPool->entityAt(i);
        auto rest =
          std::make_tuple(std::get<ComponentPool<Rest>*>(restPools)->get(e)...);
        if ((... && (std::get<Rest*>(rest) != nullptr))) {
          func(e, (*firstPool)[i], (*std::get<Rest*>(rest))...);
        }
      }
    }
  }

  // Get the component from the pool, returns nullptr if not found
  template<typename T>
  T* getComponent(Entity entity)
  {
    const std::size_t typeID = getComponentTypeID<T>();
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

  // Tracks which components each entity has
  std::array<Signature, MAX_ENTITIES> m_entityComponentMasks;

  size_t m_entityCount{ 1 };
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
