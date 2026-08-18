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

TEST_F(ECSManagerTest, AddComponentsByType)
{
  Entity entity = manager->createEntity("MultiComponentEntity");

  auto& posComp = manager->emplaceComponent<PositionComponent>(entity);
  posComp.position = glm::vec3(1.0f, 2.0f, 3.0f);
  posComp.scale = glm::vec3(2.0f, 2.0f, 2.0f);

  auto& animComp = manager->emplaceComponent<AnimationComponent>(entity);
  animComp.currentTime = 1.5f;
  animComp.isPlaying = false;
  animComp.animationIndex = 2;

  auto* retrievedPos = manager->getComponent<PositionComponent>(entity);
  ASSERT_NE(retrievedPos, nullptr);
  EXPECT_EQ(retrievedPos->position, glm::vec3(1.0f, 2.0f, 3.0f));
  EXPECT_EQ(retrievedPos->scale, glm::vec3(2.0f, 2.0f, 2.0f));

  auto* retrievedAnim = manager->getComponent<AnimationComponent>(entity);
  ASSERT_NE(retrievedAnim, nullptr);
  EXPECT_FLOAT_EQ(retrievedAnim->currentTime, 1.5f);
  EXPECT_FALSE(retrievedAnim->isPlaying);
  EXPECT_EQ(retrievedAnim->animationIndex, 2);
}

TEST_F(ECSManagerTest, ComponentViews)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  // Entity1: Position only
  manager->emplaceComponent<PositionComponent>(entity1);

  // Entity2: Position + Animation
  manager->emplaceComponent<PositionComponent>(entity2);
  manager->emplaceComponent<AnimationComponent>(entity2);

  // Entity3: Animation only
  manager->emplaceComponent<AnimationComponent>(entity3);

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

  std::shared_ptr<PointLight> pLight = manager->setupPointLight(
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
    manager->setupDirectionalLight(entity, color, ambient, direction);

  ASSERT_NE(dLight, nullptr);
  EXPECT_EQ(dLight->color, color);
  EXPECT_EQ(dLight->intensity, ambient);
  EXPECT_EQ(dLight->direction, direction);

  // Update directional light
  glm::vec3 newColor(0.5f, 0.5f, 0.5f);
  float newAmbient = 0.3f;
  glm::vec3 newDirection(0.0f, -1.0f, 1.0f);

  manager->updateDirLight(newColor, newAmbient, newDirection);

  EXPECT_EQ(dLight->color, newColor);
  EXPECT_EQ(dLight->intensity, newAmbient);
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

// Entity Stress Tests
class EntityStressTest : public ::testing::Test
{
protected:
  void SetUp() override { manager = &ECSManager::getInstance(); }

  void TearDown() override { manager->reset(); }

  ECSManager* manager;
};

TEST_F(EntityStressTest, BulkCreateAndDestroy)
{
  // Covers, at one representative N (staying safely within
  // MAX_ENTITIES=1000): sequential ID assignment across a large batch,
  // component views at scale, and that destroying every entity cleans up
  // both the name map and every component pool.
  const int NUM_ENTITIES = 500;
  std::vector<Entity> entities;

  for (int i = 0; i < NUM_ENTITIES; ++i) {
    Entity entity = manager->createEntity("Entity_" + std::to_string(i));
    entities.push_back(entity);

    EXPECT_NE(entity, 0);
    if (i > 0) {
      EXPECT_EQ(entity, entities[i - 1] + 1); // IDs should be sequential
    }

    // Add components to test component pool views at scale
    if (i % 2 == 0) { // Every other entity
      auto& posComp = manager->emplaceComponent<PositionComponent>(entity);
      posComp.position = glm::vec3(i, i, i);
    }
    if (i % 3 == 0) { // Every third entity
      auto& animComp = manager->emplaceComponent<AnimationComponent>(entity);
      animComp.animationIndex = i;
    }
  }

  EXPECT_EQ(entities.size(), NUM_ENTITIES);
  EXPECT_EQ(manager->getEntityName(entities[0]), "Entity_0");
  EXPECT_EQ(manager->getEntityName(entities[NUM_ENTITIES - 1]),
            "Entity_" + std::to_string(NUM_ENTITIES - 1));

  auto positionView = manager->view<PositionComponent>();
  auto animationView = manager->view<AnimationComponent>();
  int expectedPosCount = (NUM_ENTITIES + 1) / 2;
  int expectedAnimCount = (NUM_ENTITIES + 2) / 3;
  EXPECT_EQ(positionView.size(), expectedPosCount);
  EXPECT_EQ(animationView.size(), expectedAnimCount);

  // Destroy all entities and verify full cleanup
  for (Entity entity : entities) {
    EXPECT_NE(manager->getEntityName(entity), "");

    manager->destroyEntity(entity);

    EXPECT_EQ(manager->getEntityName(entity), "");
    EXPECT_EQ(manager->getComponent<PositionComponent>(entity), nullptr);
    EXPECT_EQ(manager->getComponent<AnimationComponent>(entity), nullptr);
  }

  EXPECT_EQ(manager->view<PositionComponent>().size(), 0u);
  EXPECT_EQ(manager->view<AnimationComponent>().size(), 0u);
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
  auto& posComp = manager->emplaceComponent<PositionComponent>(lastValidEntity);
  posComp.position = glm::vec3(999, 999, 999);

  auto* retrievedComponent =
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

// each<>() / hasComponent<>() / removal-reflection tests
class ECSIterationTest : public ::testing::Test
{
protected:
  void SetUp() override { manager = &ECSManager::getInstance(); }

  void TearDown() override { manager->reset(); }

  ECSManager* manager;
};

TEST_F(ECSIterationTest, EachSingleType)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  manager->emplaceComponent<PositionComponent>(entity1).position =
    glm::vec3(1.0f, 0.0f, 0.0f);
  manager->emplaceComponent<PositionComponent>(entity2).position =
    glm::vec3(2.0f, 0.0f, 0.0f);
  manager->emplaceComponent<PositionComponent>(entity3).position =
    glm::vec3(3.0f, 0.0f, 0.0f);

  std::set<Entity> visited;
  manager->each<PositionComponent>(
    [&](Entity e, PositionComponent& pos) {
      visited.insert(e);
      // Mutate through the reference each<>() hands back
      pos.position.y = pos.position.x * 10.0f;
    });

  EXPECT_EQ(visited, (std::set<Entity>{ entity1, entity2, entity3 }));

  // Mutation through the passed reference must persist
  EXPECT_FLOAT_EQ(
    manager->getComponent<PositionComponent>(entity1)->position.y, 10.0f);
  EXPECT_FLOAT_EQ(
    manager->getComponent<PositionComponent>(entity2)->position.y, 20.0f);
  EXPECT_FLOAT_EQ(
    manager->getComponent<PositionComponent>(entity3)->position.y, 30.0f);
}

TEST_F(ECSIterationTest, EachMultiType)
{
  Entity both1 = manager->createEntity("Both1");
  Entity both2 = manager->createEntity("Both2");
  Entity positionOnly = manager->createEntity("PositionOnly");

  manager->emplaceComponent<PositionComponent>(both1);
  manager->emplaceComponent<AnimationComponent>(both1).animationIndex = 1;

  manager->emplaceComponent<PositionComponent>(both2);
  manager->emplaceComponent<AnimationComponent>(both2).animationIndex = 2;

  // Has the driving (First) component but not the Rest component — must be
  // skipped even though it's in PositionComponent's pool.
  manager->emplaceComponent<PositionComponent>(positionOnly);

  std::set<Entity> visited;
  manager->each<PositionComponent, AnimationComponent>(
    [&](Entity e, PositionComponent&, AnimationComponent& anim) {
      visited.insert(e);
      anim.currentTime = 5.0f;
    });

  EXPECT_EQ(visited, (std::set<Entity>{ both1, both2 }));
  EXPECT_FLOAT_EQ(
    manager->getComponent<AnimationComponent>(both1)->currentTime, 5.0f);
  EXPECT_FLOAT_EQ(
    manager->getComponent<AnimationComponent>(both2)->currentTime, 5.0f);
}

TEST_F(ECSIterationTest, EachSkipsWhenPoolMissing)
{
  // No LightingComponent has ever been emplaced in this fresh (reset) ECS,
  // so its pool doesn't exist yet — each<>() must no-op rather than crash.
  manager->createEntity("NoLighting");

  int callCount = 0;
  manager->each<LightingComponent>(
    [&](Entity, LightingComponent&) { ++callCount; });
  EXPECT_EQ(callCount, 0);

  // Same for the multi-type overload when the Rest pool is missing.
  Entity entity = manager->createEntity("PositionOnly");
  manager->emplaceComponent<PositionComponent>(entity);

  callCount = 0;
  manager->each<PositionComponent, LightingComponent>(
    [&](Entity, PositionComponent&, LightingComponent&) { ++callCount; });
  EXPECT_EQ(callCount, 0);
}

TEST_F(ECSIterationTest, HasComponentContract)
{
  Entity entity = manager->createEntity("HasComponentEntity");

  EXPECT_FALSE(manager->hasComponent<PositionComponent>(entity));

  manager->emplaceComponent<PositionComponent>(entity);
  EXPECT_TRUE(manager->hasComponent<PositionComponent>(entity));
  EXPECT_FALSE(manager->hasComponent<AnimationComponent>(entity));

  manager->destroyEntity(entity);
  EXPECT_FALSE(manager->hasComponent<PositionComponent>(entity));
}

TEST_F(ECSIterationTest, ViewAndEachReflectRemoval)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  manager->emplaceComponent<PositionComponent>(entity1);
  manager->emplaceComponent<PositionComponent>(entity2);
  manager->emplaceComponent<PositionComponent>(entity3);

  EXPECT_EQ(manager->view<PositionComponent>().size(), 3u);

  manager->destroyEntity(entity2);

  auto remaining = manager->view<PositionComponent>();
  EXPECT_EQ(remaining.size(), 2u);
  EXPECT_EQ(std::count(remaining.begin(), remaining.end(), entity2), 0);

  std::set<Entity> visited;
  manager->each<PositionComponent>(
    [&](Entity e, PositionComponent&) { visited.insert(e); });
  EXPECT_EQ(visited, (std::set<Entity>{ entity1, entity3 }));
}

TEST_F(ECSIterationTest, SignatureIsolatedOnRemoval)
{
  Entity entity = manager->createEntity("MultiComponentEntity");

  manager->emplaceComponent<PositionComponent>(entity);
  manager->emplaceComponent<AnimationComponent>(entity);
  auto baseLight = std::make_shared<BaseLight>();
  manager->emplaceComponent<LightingComponent>(
    entity, baseLight, LightingComponent::TYPE::POINT);

  EXPECT_TRUE(manager->hasComponent<PositionComponent>(entity));
  EXPECT_TRUE(manager->hasComponent<AnimationComponent>(entity));
  EXPECT_TRUE(manager->hasComponent<LightingComponent>(entity));

  manager->destroyEntity(entity);

  // The whole signature bitset must clear, not just one bit.
  EXPECT_FALSE(manager->hasComponent<PositionComponent>(entity));
  EXPECT_FALSE(manager->hasComponent<AnimationComponent>(entity));
  EXPECT_FALSE(manager->hasComponent<LightingComponent>(entity));
}
