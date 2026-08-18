#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "Types/LightTypes.hpp"

class SceneManagementTest : public ::testing::Test
{
protected:
  void SetUp() override { manager = &ECSManager::getInstance(); }

  void TearDown() override
  {
    // Clean up entities created during tests
    manager->reset();
  }

  ECSManager* manager;
};

TEST_F(SceneManagementTest, DefaultEntityNaming)
{
  Entity entity = manager->createEntity();

  EXPECT_NE(entity, 0);
  EXPECT_EQ(manager->getEntityName(entity), "no_name");
}

TEST_F(SceneManagementTest, SceneResetOperation)
{
  // Create multiple entities
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  // Add components to entities
  manager->emplaceComponent<PositionComponent>(entity1);
  manager->emplaceComponent<AnimationComponent>(entity2);
  auto baseLight3 = std::make_shared<BaseLight>();
  baseLight3->color = glm::vec3(0.5f, 0.5f, 1.0f);
  manager->emplaceComponent<LightingComponent>(
    entity3, baseLight3, LightingComponent::TYPE::POINT);

  // Verify entities exist before reset
  auto allPosEntities = manager->view<PositionComponent>();
  auto allAnimEntities = manager->view<AnimationComponent>();
  auto allLightEntities = manager->view<LightingComponent>();

  EXPECT_EQ(allPosEntities.size(), 1);
  EXPECT_EQ(allAnimEntities.size(), 1);
  EXPECT_EQ(allLightEntities.size(), 1);

  // Reset scene
  manager->reset();

  // Verify all entities are gone after reset
  auto posEntitiesAfterReset = manager->view<PositionComponent>();
  auto animEntitiesAfterReset = manager->view<AnimationComponent>();
  auto lightEntitiesAfterReset = manager->view<LightingComponent>();

  EXPECT_EQ(posEntitiesAfterReset.size(), 0);
  EXPECT_EQ(animEntitiesAfterReset.size(), 0);
  EXPECT_EQ(lightEntitiesAfterReset.size(), 0);
}

TEST_F(SceneManagementTest, EntityListAccess)
{
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  // Get entities list
  auto& entities = manager->getEntities();

  // Should contain all created entities
  EXPECT_GE(entities.size(), 3);

  // Verify entities are in the list
  bool found1 =
    std::find(entities.begin(), entities.end(), entity1) != entities.end();
  bool found2 =
    std::find(entities.begin(), entities.end(), entity2) != entities.end();
  bool found3 =
    std::find(entities.begin(), entities.end(), entity3) != entities.end();

  EXPECT_TRUE(found1);
  EXPECT_TRUE(found2);
  EXPECT_TRUE(found3);
}
