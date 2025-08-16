#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ECS/ComponentPool.hpp"
#include "ECS/Components/Component.hpp"
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
  auto component = std::make_shared<PositionComponent>();
  component->position = glm::vec3(1.0f, 2.0f, 3.0f);

  pool->add(entity, component);

  auto retrieved = pool->get(entity);
  ASSERT_NE(retrieved, nullptr);
  EXPECT_EQ(retrieved->position, glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST_F(ComponentPoolTest, ComponentRetrieval)
{
  Entity entity1 = 1;
  Entity entity2 = 2;

  auto component1 = std::make_shared<PositionComponent>();
  component1->position = glm::vec3(1.0f, 0.0f, 0.0f);

  auto component2 = std::make_shared<PositionComponent>();
  component2->position = glm::vec3(0.0f, 1.0f, 0.0f);

  pool->add(entity1, component1);
  pool->add(entity2, component2);

  auto retrieved1 = pool->get(entity1);
  auto retrieved2 = pool->get(entity2);

  ASSERT_NE(retrieved1, nullptr);
  ASSERT_NE(retrieved2, nullptr);
  EXPECT_EQ(retrieved1->position, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(retrieved2->position, glm::vec3(0.0f, 1.0f, 0.0f));
}

TEST_F(ComponentPoolTest, ComponentRemoval)
{
  Entity entity = 1;
  auto component = std::make_shared<PositionComponent>();

  pool->add(entity, component);
  ASSERT_NE(pool->get(entity), nullptr);

  pool->remove(entity);
  EXPECT_EQ(pool->get(entity), nullptr);
}

TEST_F(ComponentPoolTest, ComponentOperations)
{
  Entity entity = 1;

  // Check component doesn't exist initially
  EXPECT_EQ(pool->get(entity), nullptr);

  auto component = std::make_shared<PositionComponent>();
  pool->add(entity, component);

  // Check component exists after adding
  EXPECT_NE(pool->get(entity), nullptr);
  EXPECT_EQ(pool->get(entity), component);

  pool->remove(entity);

  // Check component is removed
  EXPECT_EQ(pool->get(entity), nullptr);
}

TEST_F(ComponentPoolTest, MultipleComponents)
{
  Entity entity1 = 1;
  Entity entity2 = 2;
  Entity entity3 = 3;

  auto component1 = std::make_shared<PositionComponent>();
  auto component2 = std::make_shared<PositionComponent>();
  auto component3 = std::make_shared<PositionComponent>();

  component1->position = glm::vec3(1.0f, 0.0f, 0.0f);
  component2->position = glm::vec3(0.0f, 1.0f, 0.0f);
  component3->position = glm::vec3(0.0f, 0.0f, 1.0f);

  pool->add(entity1, component1);
  pool->add(entity2, component2);
  pool->add(entity3, component3);

  // Verify all components are stored correctly
  EXPECT_EQ(pool->get(entity1)->position, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(pool->get(entity2)->position, glm::vec3(0.0f, 1.0f, 0.0f));
  EXPECT_EQ(pool->get(entity3)->position, glm::vec3(0.0f, 0.0f, 1.0f));
}

// Test Component Base Class
class ComponentTest : public ::testing::Test
{
protected:
  void SetUp() override { component = std::make_shared<PositionComponent>(); }

  std::shared_ptr<Component> component;
};

TEST_F(ComponentTest, ComponentInstantiation)
{
  // Test that components can be created and have expected properties
  auto pos1 = std::make_shared<PositionComponent>();
  auto pos2 = std::make_shared<PositionComponent>();

  ASSERT_NE(pos1, nullptr);
  ASSERT_NE(pos2, nullptr);

  // Test they have independent state
  pos1->position = glm::vec3(1.0f, 0.0f, 0.0f);
  pos2->position = glm::vec3(0.0f, 1.0f, 0.0f);

  EXPECT_NE(pos1->position, pos2->position);
}

TEST_F(ComponentTest, ComponentPolymorphism)
{
  // Test polymorphic behavior
  std::shared_ptr<Component> basePtr = std::make_shared<PositionComponent>();
  ASSERT_NE(basePtr, nullptr);

  // Should be able to cast back
  std::shared_ptr<PositionComponent> derivedPtr =
    std::dynamic_pointer_cast<PositionComponent>(basePtr);
  ASSERT_NE(derivedPtr, nullptr);
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
  std::vector<std::shared_ptr<PositionComponent>> components;
  components.reserve(NUM_COMPONENTS);

  auto start = std::chrono::high_resolution_clock::now();

  for (int i = 0; i < NUM_COMPONENTS; ++i) {
    auto comp = std::make_shared<PositionComponent>();
    comp->position = glm::vec3(i, i + 1, i + 2);
    components.push_back(comp);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Should be able to create 1000 components in reasonable time (< 10ms)
  EXPECT_LT(duration.count(), 10000);

  // Verify components are properly created
  EXPECT_EQ(components.size(), NUM_COMPONENTS);
  EXPECT_EQ(components[0]->position, glm::vec3(0, 1, 2));
  EXPECT_EQ(components[999]->position, glm::vec3(999, 1000, 1001));
}

TEST_F(PerformanceTest, ComponentPoolPerformance)
{
  ComponentPool<PositionComponent> pool;
  const int NUM_ENTITIES =
    500; // Use fewer entities to stay within MAX_ENTITIES limit

  auto start = std::chrono::high_resolution_clock::now();

  // Add components for many entities (0-based indexing to fit within
  // MAX_ENTITIES)
  for (Entity i = 0; i < NUM_ENTITIES; ++i) {
    auto comp = std::make_shared<PositionComponent>();
    comp->position = glm::vec3(i, i * 2, i * 3);
    pool.add(i, comp);
  }

  // Retrieve all components
  for (Entity i = 0; i < NUM_ENTITIES; ++i) {
    auto comp = pool.get(i);
    ASSERT_NE(comp, nullptr);
  }

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::microseconds>(end - start);

  // Should be fast (< 50ms for 1000 entities)
  EXPECT_LT(duration.count(), 50000);
}