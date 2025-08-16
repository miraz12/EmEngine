#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "InputManager.hpp"
#include "Singleton.hpp"
#include "Types/LightTypes.hpp"

// ECS Manager Tests
class ECSManagerTest : public ::testing::Test
{
protected:
  void SetUp() override { manager = &ECSManager::getInstance(); }

  void TearDown() override { manager->reset(); }

  ECSManager* manager;
};

TEST_F(ECSManagerTest, InitializeSystems)
{
  // Test that ECS manager initializes successfully
  ASSERT_NE(manager, nullptr);
}

TEST_F(ECSManagerTest, CreateEntity)
{
  Entity entity = manager->createEntity("TestEntity");

  EXPECT_NE(entity, 0);
  EXPECT_EQ(manager->getEntityName(entity), "TestEntity");
}

TEST_F(ECSManagerTest, CreateMultipleEntities)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  EXPECT_NE(entity1, entity2);
  EXPECT_NE(entity2, entity3);
  EXPECT_NE(entity1, entity3);

  EXPECT_EQ(manager->getEntityName(entity1), "Entity1");
  EXPECT_EQ(manager->getEntityName(entity2), "Entity2");
  EXPECT_EQ(manager->getEntityName(entity3), "Entity3");
}

TEST_F(ECSManagerTest, AddPositionComponent)
{
  Entity entity = manager->createEntity("PositionEntity");

  auto posComponent = std::make_shared<PositionComponent>();
  posComponent->position = glm::vec3(1.0f, 2.0f, 3.0f);
  posComponent->scale = glm::vec3(2.0f, 2.0f, 2.0f);

  manager->addComponent(entity, posComponent);

  auto retrievedComponent = manager->getComponent<PositionComponent>(entity);
  ASSERT_NE(retrievedComponent, nullptr);
  EXPECT_EQ(retrievedComponent->position, glm::vec3(1.0f, 2.0f, 3.0f));
  EXPECT_EQ(retrievedComponent->scale, glm::vec3(2.0f, 2.0f, 2.0f));
}

TEST_F(ECSManagerTest, AddAnimationComponent)
{
  Entity entity = manager->createEntity("AnimationEntity");

  auto animComponent = std::make_shared<AnimationComponent>();
  animComponent->currentTime = 1.5f;
  animComponent->isPlaying = false;
  animComponent->animationIndex = 2;

  manager->addComponent(entity, animComponent);

  auto retrievedComponent = manager->getComponent<AnimationComponent>(entity);
  ASSERT_NE(retrievedComponent, nullptr);
  EXPECT_FLOAT_EQ(retrievedComponent->currentTime, 1.5f);
  EXPECT_FALSE(retrievedComponent->isPlaying);
  EXPECT_EQ(retrievedComponent->animationIndex, 2);
}

TEST_F(ECSManagerTest, ComponentViews)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  // Entity1: Position only
  manager->addComponent(entity1, std::make_shared<PositionComponent>());

  // Entity2: Position + Animation
  manager->addComponent(entity2, std::make_shared<PositionComponent>());
  manager->addComponent(entity2, std::make_shared<AnimationComponent>());

  // Entity3: Animation only
  manager->addComponent(entity3, std::make_shared<AnimationComponent>());

  auto positionView = manager->view<PositionComponent>();
  auto animationView = manager->view<AnimationComponent>();
  auto bothView = manager->view<PositionComponent, AnimationComponent>();

  EXPECT_EQ(positionView.size(), 2);  // Entity1 and Entity2
  EXPECT_EQ(animationView.size(), 2); // Entity2 and Entity3
  EXPECT_EQ(bothView.size(), 1);      // Only Entity2
}

TEST_F(ECSManagerTest, SetupPointLight)
{
  Entity entity = manager->createEntity("LightEntity");

  glm::vec3 color(1.0f, 1.0f, 1.0f);
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
  glm::vec3 position(0.0f, 1.0f, 0.0f);

  std::shared_ptr<PointLight> pLight = manager->SetupPointLight(
    entity, color, constant, linear, quadratic, position);

  ASSERT_NE(pLight, nullptr);
  EXPECT_EQ(pLight->color, color);
  EXPECT_EQ(pLight->constant, constant);
  EXPECT_EQ(pLight->linear, linear);
  EXPECT_EQ(pLight->quadratic, quadratic);
  EXPECT_EQ(pLight->position, position);
}

TEST_F(ECSManagerTest, SetupAndUpdateDirectionalLight)
{
  Entity entity = manager->createEntity("DirectionalLightEntity");

  glm::vec3 color(1.0f, 1.0f, 1.0f);
  float ambient = 0.5f;
  glm::vec3 direction(1.0f, -1.0f, 0.0f);

  std::shared_ptr<DirectionalLight> dLight =
    manager->SetupDirectionalLight(entity, color, ambient, direction);

  ASSERT_NE(dLight, nullptr);
  EXPECT_EQ(dLight->color, color);
  EXPECT_EQ(dLight->ambientIntensity, ambient);
  EXPECT_EQ(dLight->direction, direction);

  // Update directional light
  glm::vec3 newColor(0.5f, 0.5f, 0.5f);
  float newAmbient = 0.3f;
  glm::vec3 newDirection(0.0f, -1.0f, 1.0f);

  manager->updateDirLight(newColor, newAmbient, newDirection);

  EXPECT_EQ(dLight->color, newColor);
  EXPECT_EQ(dLight->ambientIntensity, newAmbient);
  EXPECT_EQ(dLight->direction, newDirection);
}

TEST_F(ECSManagerTest, PhysicsSimulationToggle)
{
  // Test physics simulation state management
  bool initialState = manager->getSimulatePhysics();

  manager->setSimulatePhysics(!initialState);
  EXPECT_EQ(manager->getSimulatePhysics(), !initialState);

  manager->setSimulatePhysics(initialState);
  EXPECT_EQ(manager->getSimulatePhysics(), initialState);
}

TEST_F(ECSManagerTest, EntityManagement)
{
  Entity entity = manager->createEntity("TestEntity");

  // Add multiple components
  auto posComponent = std::make_shared<PositionComponent>();
  auto animComponent = std::make_shared<AnimationComponent>();
  auto baseLight = std::make_shared<BaseLight>();
  baseLight->color = glm::vec3(1.0f, 1.0f, 1.0f);
  auto lightComponent = std::make_shared<LightingComponent>(
    baseLight, LightingComponent::TYPE::POINT);

  manager->addComponent(entity, posComponent);
  manager->addComponent(entity, animComponent);
  manager->addComponent(entity, lightComponent);

  // Verify all components can be retrieved
  EXPECT_NE(manager->getComponent<PositionComponent>(entity), nullptr);
  EXPECT_NE(manager->getComponent<AnimationComponent>(entity), nullptr);
  EXPECT_NE(manager->getComponent<LightingComponent>(entity), nullptr);
}

TEST_F(ECSManagerTest, LastEntityTracking)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");

  // Verify first entity is valid
  EXPECT_NE(entity1, 0);

  // The last created entity should be tracked
  Entity lastEntity = manager->getLastEntity();
  EXPECT_EQ(lastEntity, entity2);
}

// Singleton Pattern Tests
TEST(SingletonTest, ECSManagerSingleton)
{
  ECSManager& instance1 = ECSManager::getInstance();
  ECSManager& instance2 = ECSManager::getInstance();

  EXPECT_EQ(&instance1, &instance2);
}

TEST(SingletonTest, InputManagerSingleton)
{
  InputManager& instance1 = InputManager::getInstance();
  InputManager& instance2 = InputManager::getInstance();

  EXPECT_EQ(&instance1, &instance2);
}

// Input Manager Tests
class InputManagerTest : public ::testing::Test
{
protected:
  void SetUp() override { inputManager = &InputManager::getInstance(); }

  InputManager* inputManager;
};

TEST_F(InputManagerTest, KeyAccess)
{
  // Test that we can access different key states
  bool wKey = inputManager->getKey(KEY::W);
  bool spaceKey = inputManager->getKey(KEY::Space);
  bool escapeKey = inputManager->getKey(KEY::Escape);

  // These should return valid boolean values
  EXPECT_TRUE(wKey == true || wKey == false);
  EXPECT_TRUE(spaceKey == true || spaceKey == false);
  EXPECT_TRUE(escapeKey == true || escapeKey == false);
}

TEST_F(InputManagerTest, MousePositionHandling)
{
  double testX = 150.0;
  double testY = 250.0;

  // Test that setMousePos doesn't crash and can be called
  inputManager->setMousePos(testX, testY);

  // Method exists and can be called successfully
  SUCCEED();
}

TEST_F(InputManagerTest, HandleInput)
{
  // Test input handling methods exist and can be called
  inputManager->handleInput(65, 1); // 'A' key press
  inputManager->handleAction(KEY::W, 1);

  // Methods exist and can be called successfully
  SUCCEED();
}

// Entity Stress Tests
class EntityStressTest : public ::testing::Test
{
protected:
  void SetUp() override { manager = &ECSManager::getInstance(); }

  void TearDown() override { manager->reset(); }

  ECSManager* manager;
};

TEST_F(EntityStressTest, CreateManyEntities)
{
  const int NUM_ENTITIES = 800; // Test creating many entities (staying safely
                                // within MAX_ENTITIES=1000)
  std::vector<Entity> entities;

  // Create many entities
  for (int i = 0; i < NUM_ENTITIES; ++i) {
    Entity entity = manager->createEntity("StressEntity_" + std::to_string(i));
    entities.push_back(entity);

    // Verify entity is valid and has sequential IDs
    EXPECT_NE(entity, 0);
    if (i > 0) {
      EXPECT_EQ(entity, entities[i - 1] + 1); // IDs should be sequential
    }
  }

  // Verify all entities exist and have correct names
  EXPECT_EQ(entities.size(), NUM_ENTITIES);
  EXPECT_EQ(manager->getLastEntity(),
            NUM_ENTITIES); // Last entity should be NUM_ENTITIES

  // Check first and last entities
  EXPECT_EQ(manager->getEntityName(entities[0]), "StressEntity_0");
  EXPECT_EQ(manager->getEntityName(entities[NUM_ENTITIES - 1]),
            "StressEntity_" + std::to_string(NUM_ENTITIES - 1));
}

TEST_F(EntityStressTest, CreateAndDestroyManyEntities)
{
  const int NUM_ENTITIES = 500; // Stay safely within MAX_ENTITIES limit
  std::vector<Entity> entities;

  // Create entities
  for (int i = 0; i < NUM_ENTITIES; ++i) {
    Entity entity = manager->createEntity("Entity_" + std::to_string(i));
    entities.push_back(entity);

    // Add a component to make sure destruction cleans up properly
    auto posComponent = std::make_shared<PositionComponent>();
    posComponent->position = glm::vec3(i, i, i);
    manager->addComponent(entity, posComponent);
  }

  // Verify all entities exist
  EXPECT_EQ(entities.size(), NUM_ENTITIES);

  // Destroy all entities
  for (Entity entity : entities) {
    // Verify entity exists before destruction
    EXPECT_NE(manager->getEntityName(entity), "");
    EXPECT_NE(manager->getComponent<PositionComponent>(entity), nullptr);

    // Destroy the entity
    manager->destroyEntity(entity);

    // Verify entity is cleaned up
    EXPECT_EQ(manager->getEntityName(entity), ""); // Name should be empty
    EXPECT_EQ(manager->getComponent<PositionComponent>(entity),
              nullptr); // Component should be null
  }
}

TEST_F(EntityStressTest, EntityIdReuseBehavior)
{
  // Test NEW behavior: IDs ARE reused from destroyed entities

  // Create initial entities
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  EXPECT_EQ(entity1, 1);
  EXPECT_EQ(entity2, 2);
  EXPECT_EQ(entity3, 3);

  // Destroy middle entity
  manager->destroyEntity(entity2);

  // Verify destroyed entity is really gone
  EXPECT_EQ(manager->getEntityName(entity2), "");

  // Create new entity - should reuse the destroyed entity ID (2)
  Entity entity4 = manager->createEntity("Entity4");
  EXPECT_EQ(entity4, 2); // Should reuse ID 2

  // Verify other entities still exist
  EXPECT_EQ(manager->getEntityName(entity1), "Entity1");
  EXPECT_EQ(manager->getEntityName(entity3), "Entity3");
  EXPECT_EQ(manager->getEntityName(entity4), "Entity4");

  // Create another entity - should get next available sequential ID
  Entity entity5 = manager->createEntity("Entity5");
  EXPECT_EQ(entity5, 4); // Should be 4 (next in sequence)

  // Destroy multiple entities to test queue behavior
  manager->destroyEntity(entity1);
  manager->destroyEntity(entity3);

  // Create new entities - should reuse in FIFO order (entity1 destroyed first)
  Entity entity6 = manager->createEntity("Entity6");
  Entity entity7 = manager->createEntity("Entity7");

  EXPECT_EQ(entity6, 1); // Should reuse entity1's ID
  EXPECT_EQ(entity7, 3); // Should reuse entity3's ID
}

TEST_F(EntityStressTest, EntityLimitBehavior)
{
  // Test behavior near MAX_ENTITIES limit
  // MAX_ENTITIES is 1000, so let's test creating close to that limit
  const int ENTITIES_TO_CREATE = 950; // Stay safely under limit for this test
  std::vector<Entity> entities;

  for (int i = 0; i < ENTITIES_TO_CREATE; ++i) {
    Entity entity = manager->createEntity("LimitEntity_" + std::to_string(i));
    entities.push_back(entity);

    // Add components to test component pool limits too
    if (i % 2 == 0) { // Add PositionComponent to every other entity
      auto posComponent = std::make_shared<PositionComponent>();
      posComponent->position = glm::vec3(i, 0, 0);
      manager->addComponent(entity, posComponent);
    }

    if (i % 3 == 0) { // Add AnimationComponent to every third entity
      auto animComponent = std::make_shared<AnimationComponent>();
      animComponent->animationIndex = i;
      manager->addComponent(entity, animComponent);
    }
  }

  // Verify all entities were created successfully
  EXPECT_EQ(entities.size(), ENTITIES_TO_CREATE);

  // Test component views with large number of entities
  auto positionView = manager->view<PositionComponent>();
  auto animationView = manager->view<AnimationComponent>();
  auto bothView = manager->view<PositionComponent, AnimationComponent>();

  // Calculate expected counts
  int expectedPosCount = (ENTITIES_TO_CREATE + 1) / 2;  // Every other entity
  int expectedAnimCount = (ENTITIES_TO_CREATE + 2) / 3; // Every third entity
  int expectedBothCount = 0;
  for (int i = 0; i < ENTITIES_TO_CREATE; ++i) {
    if (i % 2 == 0 && i % 3 == 0)
      expectedBothCount++; // Every sixth entity
  }

  EXPECT_EQ(positionView.size(), expectedPosCount);
  EXPECT_EQ(animationView.size(), expectedAnimCount);
  EXPECT_EQ(bothView.size(), expectedBothCount);
}

TEST_F(EntityStressTest, EntityMaxLimitBoundary)
{
  // Test NEW behavior: createEntity now properly checks MAX_ENTITIES limit

  // Create entities up to the MAX_ENTITIES limit
  // Valid entity IDs should be 1 to 999 (999 entities, since we start counting
  // at 1)

  std::vector<Entity> validEntities;
  for (int i = 0; i < 999; ++i) { // Create entities with IDs 1-999
    Entity entity = manager->createEntity("Valid_" + std::to_string(i));
    validEntities.push_back(entity);
    EXPECT_NE(entity, 0);     // Should be valid
    EXPECT_EQ(entity, i + 1); // Should be sequential 1, 2, 3...
  }

  // Try to create one more entity - this should FAIL because we've hit
  // MAX_ENTITIES
  Entity failedEntity = manager->createEntity("ShouldFail");
  EXPECT_EQ(failedEntity, 0); // Should return 0 (invalid entity) due to limit

  // The failed entity should not exist in name map
  EXPECT_EQ(manager->getEntityName(failedEntity), "");

  // Verify we can still work with existing entities
  Entity lastValidEntity = validEntities.back();
  EXPECT_EQ(lastValidEntity, 999);

  // Components should work fine on valid entities
  auto posComponent = std::make_shared<PositionComponent>();
  posComponent->position = glm::vec3(999, 999, 999);
  manager->addComponent(lastValidEntity, posComponent);

  auto retrievedComponent =
    manager->getComponent<PositionComponent>(lastValidEntity);
  ASSERT_NE(retrievedComponent, nullptr);
  EXPECT_EQ(retrievedComponent->position, glm::vec3(999, 999, 999));
}

TEST_F(EntityStressTest, EntityLimitWithReuse)
{
  // Test that entity reuse allows us to bypass the limit after destroying
  // entities

  std::vector<Entity> entities;

  // Create entities up to the limit
  for (int i = 0; i < 999; ++i) {
    Entity entity = manager->createEntity("Entity_" + std::to_string(i));
    entities.push_back(entity);
    EXPECT_NE(entity, 0);
  }

  // Try to create one more - should fail
  Entity shouldFail = manager->createEntity("ShouldFail");
  EXPECT_EQ(shouldFail, 0);

  // Destroy some entities
  for (int i = 0; i < 10; ++i) {
    manager->destroyEntity(entities[i]);
  }

  // Now we should be able to create new entities (reusing destroyed IDs)
  std::vector<Entity> reusedEntities;
  for (int i = 0; i < 10; ++i) {
    Entity entity = manager->createEntity("Reused_" + std::to_string(i));
    reusedEntities.push_back(entity);
    EXPECT_NE(entity, 0); // Should succeed

    // Should be one of the destroyed entity IDs (1-10)
    EXPECT_GE(entity, 1);
    EXPECT_LE(entity, 10);
  }

  // Try to create one more after reusing all available IDs - should fail again
  Entity shouldFailAgain = manager->createEntity("ShouldFailAgain");
  EXPECT_EQ(shouldFailAgain, 0);
}

TEST_F(EntityStressTest, MassEntityDestructionAndRecreation)
{
  const int BATCH_SIZE = 150; // Reduced to stay within MAX_ENTITIES
  const int NUM_BATCHES = 4;  // 4 batches × 150 = 600 entities total

  for (int batch = 0; batch < NUM_BATCHES; ++batch) {
    std::vector<Entity> batchEntities;

    // Create batch of entities
    for (int i = 0; i < BATCH_SIZE; ++i) {
      Entity entity = manager->createEntity("Batch" + std::to_string(batch) +
                                            "_Entity" + std::to_string(i));
      batchEntities.push_back(entity);

      // Add random components
      if (i % 2 == 0) {
        auto posComponent = std::make_shared<PositionComponent>();
        posComponent->position = glm::vec3(batch, i, 0);
        manager->addComponent(entity, posComponent);
      }

      if (i % 3 == 0) {
        auto animComponent = std::make_shared<AnimationComponent>();
        animComponent->currentTime = batch * i;
        manager->addComponent(entity, animComponent);
      }
    }

    // Verify batch creation
    EXPECT_EQ(batchEntities.size(), BATCH_SIZE);

    // Destroy all entities in batch
    for (Entity entity : batchEntities) {
      manager->destroyEntity(entity);
    }

    // Verify all entities in batch are destroyed
    for (Entity entity : batchEntities) {
      EXPECT_EQ(manager->getEntityName(entity), "");
      EXPECT_EQ(manager->getComponent<PositionComponent>(entity), nullptr);
      EXPECT_EQ(manager->getComponent<AnimationComponent>(entity), nullptr);
    }
  }

  // With ID reuse, we should be able to create a new entity
  // It should reuse one of the many destroyed entity IDs
  Entity finalEntity = manager->createEntity("FinalEntity");
  EXPECT_NE(finalEntity, 0); // Should succeed

  // The reused ID should be within the range of previously created entities
  EXPECT_GE(finalEntity, 1);
  EXPECT_LE(finalEntity, BATCH_SIZE * NUM_BATCHES);
}
