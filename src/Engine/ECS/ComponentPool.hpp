#ifndef COMPONENTPOOL_H_
#define COMPONENTPOOL_H_

#include <cassert>
#include <limits>
#include <vector>

// A pool for a specific component type
class IComponentPool
{
public:
  virtual ~IComponentPool() = default;
  virtual void entityDestroyed(Entity entity) = 0;
  virtual void clear() = 0;
};

template<typename T>
class ComponentPool : public IComponentPool
{
  static constexpr size_t INVALID = std::numeric_limits<size_t>::max();

public:
  ComponentPool() { m_entityToIndex.resize(MAX_ENTITIES, INVALID); }

  template<typename... Args>
  T& emplace(Entity entity, Args&&... args)
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    assert(m_entityToIndex[entity] == INVALID && "Component already exists");

    size_t newIndex = m_components.size();
    m_components.emplace_back(std::forward<Args>(args)...);
    m_indexToEntity.push_back(entity);
    m_entityToIndex[entity] = newIndex;

    return m_components.back();
  }

  // Swap-and-pop removal: O(1) by moving the last element into the gap
  void remove(Entity entity)
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    size_t removedIndex = m_entityToIndex[entity];
    if (removedIndex == INVALID)
      return;

    size_t lastIndex = m_components.size() - 1;

    if (removedIndex != lastIndex) {
      Entity lastEntity = m_indexToEntity[lastIndex];
      m_components[removedIndex] = std::move(m_components[lastIndex]);
      m_indexToEntity[removedIndex] = lastEntity;
      m_entityToIndex[lastEntity] = removedIndex;
    }

    m_components.pop_back();
    m_indexToEntity.pop_back();
    m_entityToIndex[entity] = INVALID;
  }

  T* get(Entity entity)
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    size_t index = m_entityToIndex[entity];
    if (index == INVALID)
      return nullptr;
    return &m_components[index];
  }

  const T* get(Entity entity) const
  {
    assert(entity < MAX_ENTITIES && "Entity out of range");
    size_t index = m_entityToIndex[entity];
    if (index == INVALID)
      return nullptr;
    return &m_components[index];
  }

  bool has(Entity entity) const
  {
    return entity < MAX_ENTITIES && m_entityToIndex[entity] != INVALID;
  }

  void entityDestroyed(Entity entity) override
  {
    if (entity < MAX_ENTITIES && m_entityToIndex[entity] != INVALID) {
      remove(entity);
    }
  }

  void clear() override
  {
    m_components.clear();
    m_indexToEntity.clear();
    std::fill(m_entityToIndex.begin(), m_entityToIndex.end(), INVALID);
  }

  // Direct iteration
  size_t size() const { return m_components.size(); }
  T& operator[](size_t index) { return m_components[index]; }
  const T& operator[](size_t index) const { return m_components[index]; }
  Entity entityAt(size_t index) const { return m_indexToEntity[index]; }

  T* begin() { return m_components.data(); }
  T* end() { return m_components.data() + m_components.size(); }
  const T* begin() const { return m_components.data(); }
  const T* end() const { return m_components.data() + m_components.size(); }

private:
  std::vector<T> m_components;         // Components stored contiguously
  std::vector<Entity> m_indexToEntity; // Dense index -> entity ID
  std::vector<size_t>
    m_entityToIndex; // Entity ID -> dense index (INVALID if absent)
};

#endif // COMPONENTPOOL_H_
