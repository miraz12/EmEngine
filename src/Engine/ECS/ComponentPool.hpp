#ifndef COMPONENTPOOL_H_
#define COMPONENTPOOL_H_

#include <array>
#include <cassert>
#include <memory>
#include <vector>
#include "Components/Component.hpp"

// A pool for a specific component type
class IComponentPool
{
public:
  virtual ~IComponentPool() = default;
  virtual void entityDestroyed(Entity entity) = 0;
};

template<typename T>
class ComponentPool : public IComponentPool
{
public:
  ComponentPool()
  {
    // Pre-allocate component array with nullptr
    m_components.resize(MAX_ENTITIES, nullptr);
  }

  void add(Entity entity, std::shared_ptr<T> component)
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    m_components[entity] = component;
  }

  void remove(Entity entity)
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    m_components[entity] = nullptr;
  }

  std::shared_ptr<T> get(Entity entity)
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    return m_components[entity];
  }

  void entityDestroyed(Entity entity) override
  {
    if (entity < MAX_ENTITIES) {
      m_components[entity] = nullptr;
    }
  }

private:
  // Components stored contiguously by entity ID
  std::vector<std::shared_ptr<T>> m_components;
};

#endif // COMPONENTPOOL_H_
