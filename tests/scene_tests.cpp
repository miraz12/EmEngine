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

TEST_F(SceneManagementTest, EntityCreation)
{
  Entity entity = manager->createEntity("TestEntity");

  EXPECT_NE(entity, 0); // Entity should have valid ID
  EXPECT_EQ(manager->getEntityName(entity), "TestEntity");
}

TEST_F(SceneManagementTest, EntityNaming)
{
  Entity entity1 = manager->createEntity("FirstEntity");
  Entity entity2 = manager->createEntity("SecondEntity");
  Entity entity3 = manager->createEntity("ThirdEntity");

  EXPECT_EQ(manager->getEntityName(entity1), "FirstEntity");
  EXPECT_EQ(manager->getEntityName(entity2), "SecondEntity");
  EXPECT_EQ(manager->getEntityName(entity3), "ThirdEntity");

  EXPECT_NE(entity1, entity2);
  EXPECT_NE(entity2, entity3);
  EXPECT_NE(entity1, entity3);
}

TEST_F(SceneManagementTest, DefaultEntityNaming)
{
  Entity entity = manager->createEntity();

  EXPECT_NE(entity, 0);
  EXPECT_EQ(manager->getEntityName(entity), "no_name");
}

TEST_F(SceneManagementTest, ComponentAdditionAndRetrieval)
{
  Entity entity = manager->createEntity("ComponentEntity");

  auto& posComp = manager->emplaceComponent<PositionComponent>(entity);
  posComp.position = glm::vec3(1.0f, 2.0f, 3.0f);
  posComp.rotation = glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f);
  posComp.scale = glm::vec3(2.0f, 2.0f, 2.0f);

  auto* retrievedComponent = manager->getComponent<PositionComponent>(entity);
  ASSERT_NE(retrievedComponent, nullptr);
  EXPECT_EQ(retrievedComponent->position, glm::vec3(1.0f, 2.0f, 3.0f));
  EXPECT_EQ(retrievedComponent->scale, glm::vec3(2.0f, 2.0f, 2.0f));
}

TEST_F(SceneManagementTest, MultipleComponentsPerEntity)
{
  Entity entity = manager->createEntity("MultiCompEntity");

  // Add position component
  auto& posComp = manager->emplaceComponent<PositionComponent>(entity);
  posComp.position = glm::vec3(0.0f, 5.0f, 0.0f);

  // Add animation component
  auto& animComp = manager->emplaceComponent<AnimationComponent>(entity);
  animComp.currentTime = 1.2f;
  animComp.isPlaying = true;
  animComp.animationIndex = 3;

  // Add lighting component
  auto baseLight1 = std::make_shared<BaseLight>();
  baseLight1->color = glm::vec3(1.0f, 1.0f, 1.0f);
  manager->emplaceComponent<LightingComponent>(
    entity, baseLight1, LightingComponent::TYPE::POINT);

  // Retrieve all components
  auto* posResult = manager->getComponent<PositionComponent>(entity);
  auto* animResult = manager->getComponent<AnimationComponent>(entity);
  auto* lightResult = manager->getComponent<LightingComponent>(entity);

  ASSERT_NE(posResult, nullptr);
  ASSERT_NE(animResult, nullptr);
  ASSERT_NE(lightResult, nullptr);

  EXPECT_EQ(posResult->position, glm::vec3(0.0f, 5.0f, 0.0f));
  EXPECT_FLOAT_EQ(animResult->currentTime, 1.2f);
  EXPECT_EQ(animResult->animationIndex, 3);
  EXPECT_TRUE(animResult->isPlaying);
}

TEST_F(SceneManagementTest, EntityViews)
{
  // Create entities with different component combinations
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");
  Entity entity4 = manager->createEntity("Entity4");

  // Entity1: Position only
  manager->emplaceComponent<PositionComponent>(entity1);

  // Entity2: Position + Animation
  manager->emplaceComponent<PositionComponent>(entity2);
  manager->emplaceComponent<AnimationComponent>(entity2);

  // Entity3: Animation only
  manager->emplaceComponent<AnimationComponent>(entity3);

  // Entity4: Position + Lighting
  manager->emplaceComponent<PositionComponent>(entity4);
  auto baseLight2 = std::make_shared<BaseLight>();
  baseLight2->color = glm::vec3(0.8f, 0.8f, 0.8f);
  manager->emplaceComponent<LightingComponent>(
    entity4, baseLight2, LightingComponent::TYPE::DIRECTIONAL);

  // Test views
  auto positionView = manager->view<PositionComponent>();
  auto animationView = manager->view<AnimationComponent>();
  auto posAnimView = manager->view<PositionComponent, AnimationComponent>();
  auto posLightView = manager->view<PositionComponent, LightingComponent>();

  EXPECT_EQ(positionView.size(), 3);  // Entity1, Entity2, Entity4
  EXPECT_EQ(animationView.size(), 2); // Entity2, Entity3
  EXPECT_EQ(posAnimView.size(), 1);   // Only Entity2
  EXPECT_EQ(posLightView.size(), 1);  // Only Entity4
}

TEST_F(SceneManagementTest, LightSetup)
{
  Entity lightEntity = manager->createEntity("LightEntity");

  // Test point light setup
  glm::vec3 color(1.0f, 0.8f, 0.6f);
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
  glm::vec3 position(2.0f, 3.0f, 1.0f);

  std::shared_ptr<PointLight> pointLight = manager->setupPointLight(
    lightEntity, color, constant, linear, quadratic, position);

  ASSERT_NE(pointLight, nullptr);
  EXPECT_EQ(pointLight->color, color);
  EXPECT_FLOAT_EQ(pointLight->constant, constant);
  EXPECT_FLOAT_EQ(pointLight->linear, linear);
  EXPECT_FLOAT_EQ(pointLight->quadratic, quadratic);
  EXPECT_EQ(pointLight->position, position);

  // Verify the lighting component was added to the entity
  auto* lightingComp = manager->getComponent<LightingComponent>(lightEntity);
  ASSERT_NE(lightingComp, nullptr);
  EXPECT_EQ(lightingComp->type, LightingComponent::TYPE::POINT);
}

TEST_F(SceneManagementTest, DirectionalLightSetup)
{
  Entity lightEntity = manager->createEntity("DirectionalLight");

  glm::vec3 color(1.0f, 1.0f, 0.9f);
  float ambient = 0.4f;
  glm::vec3 direction(-0.5f, -1.0f, -0.3f);

  std::shared_ptr<DirectionalLight> dirLight =
    manager->setupDirectionalLight(lightEntity, color, ambient, direction);

  ASSERT_NE(dirLight, nullptr);
  EXPECT_EQ(dirLight->color, color);
  EXPECT_FLOAT_EQ(dirLight->intensity, ambient);
  EXPECT_EQ(dirLight->direction, direction);

  // Verify the lighting component was added to the entity
  auto* lightingComp = manager->getComponent<LightingComponent>(lightEntity);
  ASSERT_NE(lightingComp, nullptr);
  EXPECT_EQ(lightingComp->type, LightingComponent::TYPE::DIRECTIONAL);
}

TEST_F(SceneManagementTest, DirectionalLightUpdate)
{
  Entity lightEntity = manager->createEntity("DirectionalLight");

  // Initial setup
  glm::vec3 initialColor(1.0f, 1.0f, 1.0f);
  float initialAmbient = 0.3f;
  glm::vec3 initialDirection(0.0f, -1.0f, 0.0f);

  std::shared_ptr<DirectionalLight> dirLight = manager->setupDirectionalLight(
    lightEntity, initialColor, initialAmbient, initialDirection);

  // Update light properties
  glm::vec3 newColor(0.8f, 0.9f, 1.0f);
  float newAmbient = 0.5f;
  glm::vec3 newDirection(-0.2f, -0.8f, -0.6f);

  manager->updateDirLight(newColor, newAmbient, newDirection);

  EXPECT_EQ(dirLight->color, newColor);
  EXPECT_FLOAT_EQ(dirLight->intensity, newAmbient);
  EXPECT_EQ(dirLight->direction, newDirection);
}

TEST_F(SceneManagementTest, MultipleLightTypes)
{
  Entity pointLightEntity = manager->createEntity("PointLight");
  Entity dirLightEntity = manager->createEntity("DirectionalLight");

  // Setup point light
  auto pointLight = manager->setupPointLight(pointLightEntity,
                                             glm::vec3(1.0f, 0.0f, 0.0f),
                                             1.0f,
                                             0.09f,
                                             0.032f,
                                             glm::vec3(1.0f, 2.0f, 3.0f));

  // Setup directional light
  auto dirLight = manager->setupDirectionalLight(dirLightEntity,
                                                 glm::vec3(0.0f, 1.0f, 0.0f),
                                                 0.2f,
                                                 glm::vec3(-1.0f, -1.0f, 0.0f));

  ASSERT_NE(pointLight, nullptr);
  ASSERT_NE(dirLight, nullptr);

  // Verify different light types have different properties
  EXPECT_EQ(pointLight->color, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(dirLight->color, glm::vec3(0.0f, 1.0f, 0.0f));

  // Verify both entities have lighting components
  auto* pointLightComp =
    manager->getComponent<LightingComponent>(pointLightEntity);
  auto* dirLightComp = manager->getComponent<LightingComponent>(dirLightEntity);

  ASSERT_NE(pointLightComp, nullptr);
  ASSERT_NE(dirLightComp, nullptr);

  EXPECT_EQ(pointLightComp->type, LightingComponent::TYPE::POINT);
  EXPECT_EQ(dirLightComp->type, LightingComponent::TYPE::DIRECTIONAL);
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

TEST_F(SceneManagementTest, ComplexSceneSetup)
{
  // Create a complex scene with multiple entity types

  // Player entity
  Entity player = manager->createEntity("Player");
  auto& playerPos = manager->emplaceComponent<PositionComponent>(player);
  playerPos.position = glm::vec3(0.0f, 1.0f, 0.0f);
  auto& playerAnim = manager->emplaceComponent<AnimationComponent>(player);
  playerAnim.isPlaying = true;
  playerAnim.animationIndex = 1;

  // Environment objects
  std::vector<Entity> objects;
  for (int i = 0; i < 5; ++i) {
    Entity obj = manager->createEntity("Object" + std::to_string(i));
    auto& objPos = manager->emplaceComponent<PositionComponent>(obj);
    objPos.position = glm::vec3(i * 2.0f - 4.0f, 0.0f, 0.0f);
    objPos.scale = glm::vec3(0.5f + i * 0.1f);
    objects.push_back(obj);
  }

  // Lighting setup
  Entity mainLight = manager->createEntity("MainLight");
  manager->setupDirectionalLight(mainLight,
                                 glm::vec3(1.0f, 0.95f, 0.8f),
                                 0.3f,
                                 glm::vec3(-0.3f, -0.8f, -0.5f));

  Entity accentLight = manager->createEntity("AccentLight");
  manager->setupPointLight(accentLight,
                           glm::vec3(0.8f, 0.4f, 1.0f),
                           1.0f,
                           0.045f,
                           0.0075f,
                           glm::vec3(2.0f, 3.0f, -1.0f));

  // Verify scene complexity
  auto positionEntities = manager->view<PositionComponent>();
  auto animationEntities = manager->view<AnimationComponent>();
  auto lightEntities = manager->view<LightingComponent>();

  EXPECT_EQ(positionEntities.size(), 6);  // Player + 5 objects
  EXPECT_EQ(animationEntities.size(), 1); // Player only
  EXPECT_EQ(lightEntities.size(), 2);     // Main light + accent light

  // Verify specific entity properties
  EXPECT_EQ(manager->getEntityName(player), "Player");
  EXPECT_EQ(manager->getEntityName(objects[0]), "Object0");
  EXPECT_EQ(manager->getEntityName(objects[4]), "Object4");
  EXPECT_EQ(manager->getEntityName(mainLight), "MainLight");
  EXPECT_EQ(manager->getEntityName(accentLight), "AccentLight");
}

TEST_F(SceneManagementTest, EntityComponentOwnership)
{
  Entity entity = manager->createEntity("OwnershipTest");

  // Create component with specific values
  glm::vec3 testPosition(10.0f, 20.0f, 30.0f);
  auto& comp = manager->emplaceComponent<PositionComponent>(entity);
  comp.position = testPosition;

  // Get component back - should point into the dense array
  auto* retrievedComponent = manager->getComponent<PositionComponent>(entity);
  ASSERT_NE(retrievedComponent, nullptr);

  // Should point to the same storage
  EXPECT_EQ(&comp, retrievedComponent);

  // Modify through one reference
  comp.position = glm::vec3(100.0f, 200.0f, 300.0f);

  // Should reflect in the other reference
  EXPECT_EQ(retrievedComponent->position, glm::vec3(100.0f, 200.0f, 300.0f));
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
