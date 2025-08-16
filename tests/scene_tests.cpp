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

  auto posComponent = std::make_shared<PositionComponent>();
  posComponent->position = glm::vec3(1.0f, 2.0f, 3.0f);
  posComponent->rotation = glm::quat(0.7071f, 0.0f, 0.7071f, 0.0f);
  posComponent->scale = glm::vec3(2.0f, 2.0f, 2.0f);

  manager->addComponent(entity, posComponent);

  auto retrievedComponent = manager->getComponent<PositionComponent>(entity);
  ASSERT_NE(retrievedComponent, nullptr);
  EXPECT_EQ(retrievedComponent->position, glm::vec3(1.0f, 2.0f, 3.0f));
  EXPECT_EQ(retrievedComponent->scale, glm::vec3(2.0f, 2.0f, 2.0f));
}

TEST_F(SceneManagementTest, MultipleComponentsPerEntity)
{
  Entity entity = manager->createEntity("MultiCompEntity");

  // Add position component
  auto posComponent = std::make_shared<PositionComponent>();
  posComponent->position = glm::vec3(0.0f, 5.0f, 0.0f);
  manager->addComponent(entity, posComponent);

  // Add animation component
  auto animComponent = std::make_shared<AnimationComponent>();
  animComponent->currentTime = 1.2f;
  animComponent->isPlaying = true;
  animComponent->animationIndex = 3;
  manager->addComponent(entity, animComponent);

  // Add lighting component
  auto baseLight1 = std::make_shared<BaseLight>();
  baseLight1->color = glm::vec3(1.0f, 1.0f, 1.0f);
  auto lightComponent = std::make_shared<LightingComponent>(
    baseLight1, LightingComponent::TYPE::POINT);
  manager->addComponent(entity, lightComponent);

  // Retrieve all components
  auto posComp = manager->getComponent<PositionComponent>(entity);
  auto animComp = manager->getComponent<AnimationComponent>(entity);
  auto lightComp = manager->getComponent<LightingComponent>(entity);

  ASSERT_NE(posComp, nullptr);
  ASSERT_NE(animComp, nullptr);
  ASSERT_NE(lightComp, nullptr);

  EXPECT_EQ(posComp->position, glm::vec3(0.0f, 5.0f, 0.0f));
  EXPECT_FLOAT_EQ(animComp->currentTime, 1.2f);
  EXPECT_EQ(animComp->animationIndex, 3);
  EXPECT_TRUE(animComp->isPlaying);
}

TEST_F(SceneManagementTest, EntityViews)
{
  // Create entities with different component combinations
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");
  Entity entity4 = manager->createEntity("Entity4");

  // Entity1: Position only
  manager->addComponent(entity1, std::make_shared<PositionComponent>());

  // Entity2: Position + Animation
  manager->addComponent(entity2, std::make_shared<PositionComponent>());
  manager->addComponent(entity2, std::make_shared<AnimationComponent>());

  // Entity3: Animation only
  manager->addComponent(entity3, std::make_shared<AnimationComponent>());

  // Entity4: Position + Lighting
  manager->addComponent(entity4, std::make_shared<PositionComponent>());
  auto baseLight2 = std::make_shared<BaseLight>();
  baseLight2->color = glm::vec3(0.8f, 0.8f, 0.8f);
  manager->addComponent(entity4,
                        std::make_shared<LightingComponent>(
                          baseLight2, LightingComponent::TYPE::DIRECTIONAL));

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

  std::shared_ptr<PointLight> pointLight = manager->SetupPointLight(
    lightEntity, color, constant, linear, quadratic, position);

  ASSERT_NE(pointLight, nullptr);
  EXPECT_EQ(pointLight->color, color);
  EXPECT_FLOAT_EQ(pointLight->constant, constant);
  EXPECT_FLOAT_EQ(pointLight->linear, linear);
  EXPECT_FLOAT_EQ(pointLight->quadratic, quadratic);
  EXPECT_EQ(pointLight->position, position);

  // Verify the lighting component was added to the entity
  auto lightingComp = manager->getComponent<LightingComponent>(lightEntity);
  ASSERT_NE(lightingComp, nullptr);
  EXPECT_EQ(lightingComp->getType(), LightingComponent::TYPE::POINT);
}

TEST_F(SceneManagementTest, DirectionalLightSetup)
{
  Entity lightEntity = manager->createEntity("DirectionalLight");

  glm::vec3 color(1.0f, 1.0f, 0.9f);
  float ambient = 0.4f;
  glm::vec3 direction(-0.5f, -1.0f, -0.3f);

  std::shared_ptr<DirectionalLight> dirLight =
    manager->SetupDirectionalLight(lightEntity, color, ambient, direction);

  ASSERT_NE(dirLight, nullptr);
  EXPECT_EQ(dirLight->color, color);
  EXPECT_FLOAT_EQ(dirLight->ambientIntensity, ambient);
  EXPECT_EQ(dirLight->direction, direction);

  // Verify the lighting component was added to the entity
  auto lightingComp = manager->getComponent<LightingComponent>(lightEntity);
  ASSERT_NE(lightingComp, nullptr);
  EXPECT_EQ(lightingComp->getType(), LightingComponent::TYPE::DIRECTIONAL);
}

TEST_F(SceneManagementTest, DirectionalLightUpdate)
{
  Entity lightEntity = manager->createEntity("DirectionalLight");

  // Initial setup
  glm::vec3 initialColor(1.0f, 1.0f, 1.0f);
  float initialAmbient = 0.3f;
  glm::vec3 initialDirection(0.0f, -1.0f, 0.0f);

  std::shared_ptr<DirectionalLight> dirLight = manager->SetupDirectionalLight(
    lightEntity, initialColor, initialAmbient, initialDirection);

  // Update light properties
  glm::vec3 newColor(0.8f, 0.9f, 1.0f);
  float newAmbient = 0.5f;
  glm::vec3 newDirection(-0.2f, -0.8f, -0.6f);

  manager->updateDirLight(newColor, newAmbient, newDirection);

  EXPECT_EQ(dirLight->color, newColor);
  EXPECT_FLOAT_EQ(dirLight->ambientIntensity, newAmbient);
  EXPECT_EQ(dirLight->direction, newDirection);
}

TEST_F(SceneManagementTest, MultipleLightTypes)
{
  Entity pointLightEntity = manager->createEntity("PointLight");
  Entity dirLightEntity = manager->createEntity("DirectionalLight");

  // Setup point light
  auto pointLight = manager->SetupPointLight(pointLightEntity,
                                             glm::vec3(1.0f, 0.0f, 0.0f),
                                             1.0f,
                                             0.09f,
                                             0.032f,
                                             glm::vec3(1.0f, 2.0f, 3.0f));

  // Setup directional light
  auto dirLight = manager->SetupDirectionalLight(dirLightEntity,
                                                 glm::vec3(0.0f, 1.0f, 0.0f),
                                                 0.2f,
                                                 glm::vec3(-1.0f, -1.0f, 0.0f));

  ASSERT_NE(pointLight, nullptr);
  ASSERT_NE(dirLight, nullptr);

  // Verify different light types have different properties
  EXPECT_EQ(pointLight->color, glm::vec3(1.0f, 0.0f, 0.0f));
  EXPECT_EQ(dirLight->color, glm::vec3(0.0f, 1.0f, 0.0f));

  // Verify both entities have lighting components
  auto pointLightComp =
    manager->getComponent<LightingComponent>(pointLightEntity);
  auto dirLightComp = manager->getComponent<LightingComponent>(dirLightEntity);

  ASSERT_NE(pointLightComp, nullptr);
  ASSERT_NE(dirLightComp, nullptr);

  EXPECT_EQ(pointLightComp->getType(), LightingComponent::TYPE::POINT);
  EXPECT_EQ(dirLightComp->getType(), LightingComponent::TYPE::DIRECTIONAL);
}

TEST_F(SceneManagementTest, SceneResetOperation)
{
  // Create multiple entities
  Entity entity1 = manager->createEntity("Entity1");
  Entity entity2 = manager->createEntity("Entity2");
  Entity entity3 = manager->createEntity("Entity3");

  // Add components to entities
  manager->addComponent(entity1, std::make_shared<PositionComponent>());
  manager->addComponent(entity2, std::make_shared<AnimationComponent>());
  auto baseLight3 = std::make_shared<BaseLight>();
  baseLight3->color = glm::vec3(0.5f, 0.5f, 1.0f);
  manager->addComponent(entity3,
                        std::make_shared<LightingComponent>(
                          baseLight3, LightingComponent::TYPE::POINT));

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
  auto playerPos = std::make_shared<PositionComponent>();
  playerPos->position = glm::vec3(0.0f, 1.0f, 0.0f);
  auto playerAnim = std::make_shared<AnimationComponent>();
  playerAnim->isPlaying = true;
  playerAnim->animationIndex = 1;
  manager->addComponent(player, playerPos);
  manager->addComponent(player, playerAnim);

  // Environment objects
  std::vector<Entity> objects;
  for (int i = 0; i < 5; ++i) {
    Entity obj = manager->createEntity("Object" + std::to_string(i));
    auto objPos = std::make_shared<PositionComponent>();
    objPos->position = glm::vec3(i * 2.0f - 4.0f, 0.0f, 0.0f);
    objPos->scale = glm::vec3(0.5f + i * 0.1f);
    manager->addComponent(obj, objPos);
    objects.push_back(obj);
  }

  // Lighting setup
  Entity mainLight = manager->createEntity("MainLight");
  manager->SetupDirectionalLight(mainLight,
                                 glm::vec3(1.0f, 0.95f, 0.8f),
                                 0.3f,
                                 glm::vec3(-0.3f, -0.8f, -0.5f));

  Entity accentLight = manager->createEntity("AccentLight");
  manager->SetupPointLight(accentLight,
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
  auto component = std::make_shared<PositionComponent>();
  glm::vec3 testPosition(10.0f, 20.0f, 30.0f);
  component->position = testPosition;

  // Add to entity
  manager->addComponent(entity, component);

  // Get component back
  auto retrievedComponent = manager->getComponent<PositionComponent>(entity);

  // Should be the same object (shared_ptr)
  EXPECT_EQ(component.get(), retrievedComponent.get());

  // Modify through one reference
  component->position = glm::vec3(100.0f, 200.0f, 300.0f);

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