#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ECS/ComponentPool.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "InputManager.hpp"
#include "Singleton.hpp"

// Singleton pattern is covered end-to-end by the real engine singletons in
// ecs_tests.cpp (SingletonTest.ECSManagerSingleton / InputManagerSingleton) —
// no need for a synthetic TestSingleton here.

// Test Component Pool Functionality
class ComponentPoolTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    pool = std::make_unique<ComponentPool<PositionComponent>>();
  }

  std::unique_ptr<ComponentPool<PositionComponent>> pool;
};

TEST_F(ComponentPoolTest, AddGetRemoveLifecycle)
{
  Entity entity1 = 1;
  Entity entity2 = 2;

  // Doesn't exist before adding
  EXPECT_EQ(pool->get(entity1), nullptr);

  // emplace()'s returned reference is only valid until the next emplace()
  // may reallocate the pool's backing vector — set through it immediately,
  // then verify by value through get() rather than by address.
  pool->emplace(entity1).position = glm::vec3(1.0f, 2.0f, 3.0f);
  pool->emplace(entity2).position = glm::vec3(0.0f, 1.0f, 0.0f);

  // Exists after adding
  auto* retrieved1 = pool->get(entity1);
  auto* retrieved2 = pool->get(entity2);
  ASSERT_NE(retrieved1, nullptr);
  ASSERT_NE(retrieved2, nullptr);
  EXPECT_EQ(retrieved1->position, glm::vec3(1.0f, 2.0f, 3.0f));
  EXPECT_EQ(retrieved2->position, glm::vec3(0.0f, 1.0f, 0.0f));

  pool->remove(entity1);

  // Removed entity is gone, untouched entity is unaffected
  EXPECT_EQ(pool->get(entity1), nullptr);
  ASSERT_NE(pool->get(entity2), nullptr);
  EXPECT_EQ(pool->get(entity2)->position, glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST_F(ComponentPoolTest, MultipleComponents)
{
  Entity entity1 = 1;
  Entity entity2 = 2;
  Entity entity3 = 3;

  pool->emplace(entity1).position = glm::vec3(1.0f, 0.0f, 0.0f);
  pool->emplace(entity2).position = glm::vec3(0.0f, 1.0f, 0.0f);
  pool->emplace(entity3).position = glm::vec3(0.0f, 0.0f, 1.0f);

  // Verify all components are stored correctly
  EXPECT_EQ(pool->get(entity1)->position, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(pool->get(entity2)->position, glm::vec3(0.0f, 1.0f, 0.0f));
  EXPECT_EQ(pool->get(entity3)->position, glm::vec3(0.0f, 0.0f, 1.0f));
}

TEST_F(ComponentPoolTest, SwapAndPopCorrectness)
{
  Entity entity1 = 1;
  Entity entity2 = 2;
  Entity entity3 = 3;

  pool->emplace(entity1).position = glm::vec3(1.0f, 0.0f, 0.0f);
  pool->emplace(entity2).position = glm::vec3(0.0f, 2.0f, 0.0f);
  pool->emplace(entity3).position = glm::vec3(0.0f, 0.0f, 3.0f);

  EXPECT_EQ(pool->size(), 3u);

  // Remove middle entity
  pool->remove(entity2);

  EXPECT_EQ(pool->size(), 2u);
  EXPECT_EQ(pool->get(entity2), nullptr);

  // Remaining entities should still be intact
  ASSERT_NE(pool->get(entity1), nullptr);
  ASSERT_NE(pool->get(entity3), nullptr);
  EXPECT_EQ(pool->get(entity1)->position, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(pool->get(entity3)->position, glm::vec3(0.0f, 0.0f, 3.0f));
}

TEST_F(ComponentPoolTest, RemoveLastElement)
{
  // SwapAndPopCorrectness removes a middle element; this covers the other
  // branch in ComponentPool::remove — removedIndex == lastIndex, where the
  // swap-with-last step is skipped entirely.
  Entity entity1 = 1;
  Entity entity2 = 2;
  Entity entity3 = 3;

  pool->emplace(entity1).position = glm::vec3(1.0f, 0.0f, 0.0f);
  pool->emplace(entity2).position = glm::vec3(0.0f, 2.0f, 0.0f);
  pool->emplace(entity3).position = glm::vec3(0.0f, 0.0f, 3.0f);

  EXPECT_EQ(pool->size(), 3u);

  // Remove the last entity in dense order
  pool->remove(entity3);

  EXPECT_EQ(pool->size(), 2u);
  EXPECT_EQ(pool->get(entity3), nullptr);

  // The other two entities are untouched
  ASSERT_NE(pool->get(entity1), nullptr);
  ASSERT_NE(pool->get(entity2), nullptr);
  EXPECT_EQ(pool->get(entity1)->position, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(pool->get(entity2)->position, glm::vec3(0.0f, 2.0f, 0.0f));
}

TEST_F(ComponentPoolTest, RepeatedAddRemoveCycles)
{
  Entity entity = 1;

  for (int cycle = 0; cycle < 5; ++cycle) {
    EXPECT_FALSE(pool->has(entity));
    EXPECT_EQ(pool->get(entity), nullptr);

    auto& comp = pool->emplace(entity);
    comp.position = glm::vec3(static_cast<float>(cycle), 0.0f, 0.0f);

    EXPECT_TRUE(pool->has(entity));
    EXPECT_EQ(pool->size(), 1u);
    ASSERT_NE(pool->get(entity), nullptr);
    EXPECT_EQ(pool->get(entity)->position,
              glm::vec3(static_cast<float>(cycle), 0.0f, 0.0f));

    pool->remove(entity);

    EXPECT_FALSE(pool->has(entity));
    EXPECT_EQ(pool->size(), 0u);
  }
}

TEST_F(ComponentPoolTest, InterleavedAddRemoveDifferentEntities)
{
  Entity a = 1;
  Entity b = 2;
  Entity c = 3;
  Entity d = 4;

  pool->emplace(a).position = glm::vec3(1.0f, 0.0f, 0.0f);
  pool->emplace(b).position = glm::vec3(2.0f, 0.0f, 0.0f);
  pool->emplace(c).position = glm::vec3(3.0f, 0.0f, 0.0f);

  // Remove the middle entity (triggers a swap-and-pop with c moving into b's slot)
  pool->remove(b);
  EXPECT_EQ(pool->size(), 2u);
  EXPECT_FALSE(pool->has(b));

  // Add a new entity — it should land in the freed slot without disturbing
  // the survivors
  pool->emplace(d).position = glm::vec3(4.0f, 0.0f, 0.0f);
  EXPECT_EQ(pool->size(), 3u);

  // Remove the first entity added (triggers another swap-and-pop)
  pool->remove(a);
  EXPECT_EQ(pool->size(), 2u);
  EXPECT_FALSE(pool->has(a));

  // c and d must still resolve to their own data through entityAt()/get()
  ASSERT_NE(pool->get(c), nullptr);
  ASSERT_NE(pool->get(d), nullptr);
  EXPECT_EQ(pool->get(c)->position, glm::vec3(3.0f, 0.0f, 0.0f));
  EXPECT_EQ(pool->get(d)->position, glm::vec3(4.0f, 0.0f, 0.0f));

  std::set<Entity> denseEntities;
  for (size_t i = 0; i < pool->size(); ++i) {
    denseEntities.insert(pool->entityAt(i));
  }
  EXPECT_EQ(denseEntities, (std::set<Entity>{ c, d }));
}

TEST_F(ComponentPoolTest, DenseIteration)
{
  Entity entity1 = 5;
  Entity entity2 = 10;
  Entity entity3 = 15;

  pool->emplace(entity1).position = glm::vec3(5.0f, 0.0f, 0.0f);
  pool->emplace(entity2).position = glm::vec3(10.0f, 0.0f, 0.0f);
  pool->emplace(entity3).position = glm::vec3(15.0f, 0.0f, 0.0f);

  // Verify begin()/end() visits exactly 3 components
  size_t count = 0;
  float posSum = 0.0f;
  for (auto& comp : *pool) {
    posSum += comp.position.x;
    count++;
  }

  EXPECT_EQ(count, 3u);
  EXPECT_FLOAT_EQ(posSum, 30.0f); // 5 + 10 + 15
}

TEST_F(ComponentPoolTest, EntityAtMapping)
{
  Entity entity1 = 3;
  Entity entity2 = 7;

  pool->emplace(entity1);
  pool->emplace(entity2);

  // entityAt should map back to original entities
  std::set<Entity> entities;
  for (size_t i = 0; i < pool->size(); ++i) {
    entities.insert(pool->entityAt(i));
  }

  EXPECT_TRUE(entities.count(entity1));
  EXPECT_TRUE(entities.count(entity2));
}

TEST_F(ComponentPoolTest, ClearPool)
{
  pool->emplace(Entity(1));
  pool->emplace(Entity(2));
  pool->emplace(Entity(3));

  EXPECT_EQ(pool->size(), 3u);

  pool->clear();

  EXPECT_EQ(pool->size(), 0u);
  EXPECT_EQ(pool->get(Entity(1)), nullptr);
  EXPECT_EQ(pool->get(Entity(2)), nullptr);
  EXPECT_EQ(pool->get(Entity(3)), nullptr);
}

// Test Input Manager Core Functionality
class InputManagerCoreTest : public ::testing::Test
{
protected:
  void SetUp() override { inputManager = &InputManager::getInstance(); }

  InputManager* inputManager;
};

TEST_F(InputManagerCoreTest, KeyMapAccess)
{
  // Test that key map is accessible and properly sized
  EXPECT_FALSE(inputManager->m_keys.empty());

  // Test specific key access
  bool hasWKey =
    inputManager->m_keys.find(KEY::W) != inputManager->m_keys.end();
  bool hasSpaceKey =
    inputManager->m_keys.find(KEY::Space) != inputManager->m_keys.end();

  EXPECT_TRUE(hasWKey);
  EXPECT_TRUE(hasSpaceKey);
}

TEST_F(InputManagerCoreTest, ActiveKeyTracking)
{
  // Test that active key vector exists and is initially empty
  EXPECT_TRUE(inputManager->m_active.empty());

  // Test that getActive returns data pointer (may be nullptr for empty vector)
  KEY* activeKeys = inputManager->getActive();

  // m_active is empty initially, so this test verifies the API works
  // The actual population of m_active happens through GetPressed() extern
  // function
  EXPECT_EQ(inputManager->m_active.size(), 0);

  // Add a key and verify it's set in m_keys
  inputManager->handleAction(KEY::W, 1);
  EXPECT_TRUE(inputManager->getKey(KEY::W));
}

// Performance and Memory Tests
class PerformanceTest : public ::testing::Test
{};

TEST_F(PerformanceTest, ComponentCreationPerformance)
{
  const int NUM_COMPONENTS = 1000;
  ComponentPool<PositionComponent> pool;

  for (int i = 0; i < NUM_COMPONENTS; ++i) {
    // Entity IDs 0..999 (MAX_ENTITIES is 1000)
    auto& comp = pool.emplace(static_cast<Entity>(i));
    comp.position = glm::vec3(i, i + 1, i + 2);
  }

  // Verify components are properly created at scale
  EXPECT_EQ(pool.size(), NUM_COMPONENTS);
  EXPECT_EQ(pool.get(0)->position, glm::vec3(0, 1, 2));
  EXPECT_EQ(pool.get(999)->position, glm::vec3(999, 1000, 1001));
}

TEST_F(PerformanceTest, ComponentPoolPerformance)
{
  ComponentPool<PositionComponent> pool;
  const int NUM_ENTITIES = 500;

  // Add components for many entities
  for (Entity i = 0; i < NUM_ENTITIES; ++i) {
    auto& comp = pool.emplace(i);
    comp.position = glm::vec3(i, i * 2, i * 3);
  }

  // Retrieve all components at scale
  for (Entity i = 0; i < NUM_ENTITIES; ++i) {
    auto* comp = pool.get(i);
    ASSERT_NE(comp, nullptr);
    EXPECT_EQ(comp->position, glm::vec3(i, i * 2, i * 3));
  }
}
