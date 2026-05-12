#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ECS/ComponentPool.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "InputManager.hpp"
#include "Singleton.hpp"

// Test Singleton Pattern Implementation
class TestSingleton : public Singleton<TestSingleton>
{
  friend class Singleton<TestSingleton>;

private:
  TestSingleton() = default;
  int testValue = 42;

public:
  int getTestValue() const { return testValue; }
  void setTestValue(int value) { testValue = value; }
};

class SingletonPatternTest : public ::testing::Test
{
protected:
  void TearDown() override
  {
    // Reset singleton state if needed
    TestSingleton::getInstance().setTestValue(42);
  }
};

TEST_F(SingletonPatternTest, SingletonInstanceCreation)
{
  TestSingleton& instance1 = TestSingleton::getInstance();
  TestSingleton& instance2 = TestSingleton::getInstance();

  // Should be the same instance
  EXPECT_EQ(&instance1, &instance2);
}

TEST_F(SingletonPatternTest, SingletonStateConsistency)
{
  TestSingleton& instance1 = TestSingleton::getInstance();
  TestSingleton& instance2 = TestSingleton::getInstance();

  EXPECT_EQ(instance1.getTestValue(), 42);
  EXPECT_EQ(instance2.getTestValue(), 42);

  instance1.setTestValue(100);

  EXPECT_EQ(instance1.getTestValue(), 100);
  EXPECT_EQ(instance2.getTestValue(), 100);
}

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

TEST_F(ComponentPoolTest, ComponentPoolCreation)
{
  ASSERT_NE(pool, nullptr);
}

TEST_F(ComponentPoolTest, ComponentAddition)
{
  Entity entity = 1;
  auto& comp = pool->emplace(entity);
  comp.position = glm::vec3(1.0f, 2.0f, 3.0f);

  auto* retrieved = pool->get(entity);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->position, glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(ComponentPoolTest, ComponentRetrieval)
{
  Entity entity1 = 1;
  Entity entity2 = 2;

  auto& comp1 = pool->emplace(entity1);
  comp1.position = glm::vec3(1.0f, 0.0f, 0.0f);

  auto& comp2 = pool->emplace(entity2);
  comp2.position = glm::vec3(0.0f, 1.0f, 0.0f);

  auto* retrieved1 = pool->get(entity1);
  auto* retrieved2 = pool->get(entity2);

  ASSERT_NE(retrieved1, nullptr);
  ASSERT_NE(retrieved2, nullptr);
  EXPECT_EQ(retrieved1->position, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(retrieved2->position, glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST_F(ComponentPoolTest, ComponentRemoval)
{
  Entity entity = 1;
  pool->emplace(entity);
  ASSERT_NE(pool->get(entity), nullptr);

  pool->remove(entity);
  EXPECT_EQ(pool->get(entity), nullptr);
}

TEST_F(ComponentPoolTest, ComponentOperations)
{
  Entity entity = 1;

  // Check component doesn't exist initially
  EXPECT_EQ(pool->get(entity), nullptr);

  auto& comp = pool->emplace(entity);

  // Check component exists after adding
  EXPECT_NE(pool->get(entity), nullptr);
  EXPECT_EQ(pool->get(entity), &comp);

  pool->remove(entity);

  // Check component is removed
  EXPECT_EQ(pool->get(entity), nullptr);
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

// Test Component Data Structures
class ComponentTest : public ::testing::Test
{
protected:
  void SetUp() override { component = PositionComponent{}; }
  PositionComponent component;
};

TEST_F(ComponentTest, ComponentInstantiation)
{
  PositionComponent pos1;
  PositionComponent pos2;

  // Test they have independent state
  pos1.position = glm::vec3(1.0f, 0.0f, 0.0f);
  pos2.position = glm::vec3(0.0f, 1.0f, 0.0f);

  EXPECT_NE(pos1.position, pos2.position);
}

// Test Input Manager Core Functionality
class InputManagerCoreTest : public ::testing::Test
{
protected:
  void SetUp() override { inputManager = &InputManager::getInstance(); }

  InputManager* inputManager;
};

TEST_F(InputManagerCoreTest, InputManagerExists)
{
  ASSERT_NE(inputManager, nullptr);
}

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

TEST_F(InputManagerCoreTest, InputHandling)
{
  // Test that input handling methods can be called
  inputManager->handleInput(87, 1); // 'W' key
  inputManager->handleAction(KEY::W, 1);

  // Test update method
  inputManager->update(0.016f); // 60 FPS delta

  // Methods should execute without crashing
  SUCCEED();
}

// Test Entity Type
class EntityTest : public ::testing::Test
{};

TEST_F(EntityTest, EntityType)
{
  Entity entity1 = 1;
  Entity entity2 = 2;
  Entity entity3 = 0;

  EXPECT_EQ(entity1, 1);
  EXPECT_EQ(entity2, 2);
  EXPECT_EQ(entity3, 0);
}

TEST_F(EntityTest, EntityComparison)
{
  Entity entity1 = 1;
  Entity entity2 = 1;
  Entity entity3 = 2;

  EXPECT_EQ(entity1, entity2);
  EXPECT_NE(entity1, entity3);
  EXPECT_NE(entity2, entity3);
}

TEST_F(EntityTest, EntityOrdering)
{
  Entity entity1 = 1;
  Entity entity2 = 2;
  Entity entity3 = 3;

  EXPECT_LT(entity1, entity2);
  EXPECT_LT(entity2, entity3);
  EXPECT_LT(entity1, entity3);

  EXPECT_GT(entity3, entity2);
  EXPECT_GT(entity2, entity1);
  EXPECT_GT(entity3, entity1);
}

// Performance and Memory Tests
class PerformanceTest : public ::testing::Test
{};

TEST_F(PerformanceTest, ComponentCreationPerformance)
{
  const int NUM_COMPONENTS = 1000;
  ComponentPool<PositionComponent> pool;

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < NUM_COMPONENTS; ++i) {
    // Entity IDs 0..999 (MAX_ENTITIES is 1000)
    auto& comp = pool.emplace(static_cast<Entity>(i));
    comp.position = glm::vec3(i, i + 1, i + 2);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Should be able to create 1000 components in reasonable time (< 10ms)
  EXPECT_LT(duration.count(), 10000);

  // Verify components are properly created
  EXPECT_EQ(pool.size(), NUM_COMPONENTS);
  EXPECT_EQ(pool.get(0)->position, glm::vec3(0, 1, 2));
  EXPECT_EQ(pool.get(999)->position, glm::vec3(999, 1000, 1001));
}

TEST_F(PerformanceTest, ComponentPoolPerformance)
{
  ComponentPool<PositionComponent> pool;
  const int NUM_ENTITIES = 500;

  auto start = std::chrono::high_resolution_clock::now();

  // Add components for many entities
  for (Entity i = 0; i < NUM_ENTITIES; ++i) {
    auto& comp = pool.emplace(i);
    comp.position = glm::vec3(i, i * 2, i * 3);
  }

  // Retrieve all components
  for (Entity i = 0; i < NUM_ENTITIES; ++i) {
    auto* comp = pool.get(i);
    ASSERT_NE(comp, nullptr);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Should be fast (< 50ms for 500 entities)
  EXPECT_LT(duration.count(), 50000);
}
