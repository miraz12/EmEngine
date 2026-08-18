#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cstdio>
#include <filesystem>

#include "ECS/Components/CameraComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PhysicsComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "ECS/ECSManager.hpp"
#include "ECS/Systems/PhysicsSystem.hpp"
#include "SceneLoader.hpp"
#include "Types/LightTypes.hpp"

// Fixtures under tests/scenes/scene_loader_*.yaml deliberately stick to
// Pos/Phy/Lig/Cam and never use "Gra" (GraphicsComponent): this binary never
// creates a GL context, and GraphicsComponent-backed primitives build GPU
// resources at construction time (see src/Engine/Objects/Cube.cpp), which is
// unsafe here.
//
// Phy IS safe to use, but PhysicsComponent's constructor touches Jolt's
// global JPH::Factory, which needs PhysicsSystem::initialize() first.
// PhysicsSystem is a process-lifetime singleton, so this runs once for the
// whole binary via a global ::testing::Environment, not per-test.
class PhysicsSystemEnvironment : public ::testing::Environment
{
public:
  void SetUp() override
  {
    PhysicsSystem::getInstance().initialize(ECSManager::getInstance());
  }
};

static ::testing::Environment* const physicsEnv =
  ::testing::AddGlobalTestEnvironment(new PhysicsSystemEnvironment());

class SceneLoaderTest : public ::testing::Test
{
protected:
  void SetUp() override
  {
    manager = &ECSManager::getInstance();
    loader = &SceneLoader::getInstance();
  }

  void TearDown() override { manager->reset(); }

  ECSManager* manager;
  SceneLoader* loader;

  static constexpr const char* kBasicFixture =
    "tests/scenes/scene_loader_basic.yaml";
  static constexpr const char* kDefaultsFixture =
    "tests/scenes/scene_loader_defaults.yaml";
};

TEST_F(SceneLoaderTest, LoadsEntitiesAndNames)
{
  loader->init(kBasicFixture);

  EXPECT_EQ(manager->view<PositionComponent>().size(), 1u);
  EXPECT_EQ(manager->view<PhysicsComponent>().size(), 1u);
  EXPECT_EQ(manager->view<LightingComponent>().size(), 2u);
  EXPECT_EQ(manager->view<CameraComponent>().size(), 1u);

  bool foundPhysicsBox = false;
  bool foundSun = false;
  bool foundLamp = false;
  bool foundCamera = false;
  for (Entity e : manager->getEntities()) {
    std::string_view name = manager->getEntityName(e);
    foundPhysicsBox |= (name == "physics_box");
    foundSun |= (name == "sun");
    foundLamp |= (name == "lamp");
    foundCamera |= (name == "main_camera");
  }
  EXPECT_TRUE(foundPhysicsBox);
  EXPECT_TRUE(foundSun);
  EXPECT_TRUE(foundLamp);
  EXPECT_TRUE(foundCamera);
}

TEST_F(SceneLoaderTest, LoadsPositionComponent)
{
  loader->init(kBasicFixture);

  auto positionEntities = manager->view<PositionComponent>();
  ASSERT_EQ(positionEntities.size(), 1u);
  Entity entity = positionEntities[0];

  EXPECT_EQ(manager->getEntityName(entity), "physics_box");

  auto* pos = manager->getComponent<PositionComponent>(entity);
  ASSERT_NE(pos, nullptr);
  EXPECT_EQ(pos->position, glm::vec3(1.5f, 2.5f, -3.0f));
  EXPECT_EQ(pos->scale, glm::vec3(1.0f, 1.0f, 1.0f));

  // addPositionComponent builds the quaternion via glm::quat(eulerAngles),
  // so compare the rotated axes rather than raw quaternion components.
  glm::vec3 rotatedForward = pos->rotation * glm::vec3(0.0f, 0.0f, 1.0f);
  EXPECT_NEAR(rotatedForward.x, 0.0f, 0.001f);
  EXPECT_NEAR(rotatedForward.y, 0.0f, 0.001f);
  EXPECT_NEAR(rotatedForward.z, 1.0f, 0.001f);
}

TEST_F(SceneLoaderTest, LoadsPhysicsComponent)
{
  loader->init(kBasicFixture);

  auto physicsEntities = manager->view<PhysicsComponent>();
  ASSERT_EQ(physicsEntities.size(), 1u);
  Entity entity = physicsEntities[0];

  auto* phy = manager->getComponent<PhysicsComponent>(entity);
  ASSERT_NE(phy, nullptr);
  EXPECT_FLOAT_EQ(phy->getMass(), 12.5f);
  EXPECT_EQ(phy->getShapeType(), CollisionShapeType::BOX);
}

TEST_F(SceneLoaderTest, LoadsPointAndDirectionalLights)
{
  loader->init(kBasicFixture);

  LightingComponent* pointLight = nullptr;
  LightingComponent* dirLight = nullptr;
  for (Entity e : manager->getEntities()) {
    auto* light = manager->getComponent<LightingComponent>(e);
    if (!light)
      continue;
    if (light->type == LightingComponent::TYPE::POINT)
      pointLight = light;
    else if (light->type == LightingComponent::TYPE::DIRECTIONAL)
      dirLight = light;
  }

  ASSERT_NE(pointLight, nullptr);
  auto* point = static_cast<PointLight*>(pointLight->light.get());
  EXPECT_EQ(point->color, glm::vec3(0.0f, 1.0f, 0.0f));
  EXPECT_EQ(point->position, glm::vec3(1.5f, 1.0f, 0.0f));
  EXPECT_FLOAT_EQ(point->constant, 1.0f);
  EXPECT_FLOAT_EQ(point->linear, 0.7f);
  EXPECT_FLOAT_EQ(point->quadratic, 1.8f);

  ASSERT_NE(dirLight, nullptr);
  auto* dir = static_cast<DirectionalLight*>(dirLight->light.get());
  EXPECT_NEAR(dir->color.r, 0.988f, 0.001f);
  EXPECT_NEAR(dir->color.g, 0.898f, 0.001f);
  EXPECT_NEAR(dir->color.b, 0.439f, 0.001f);
  EXPECT_FLOAT_EQ(dir->intensity, 0.5f);
  EXPECT_EQ(dir->direction, glm::vec3(0.01f, -1.0f, 0.0f));
}

TEST_F(SceneLoaderTest, LoadsCameraWithExplicitValues)
{
  loader->init(kBasicFixture);

  auto cameraEntities = manager->view<CameraComponent>();
  ASSERT_EQ(cameraEntities.size(), 1u);
  auto* cam = manager->getComponent<CameraComponent>(cameraEntities[0]);
  ASSERT_NE(cam, nullptr);

  EXPECT_FLOAT_EQ(cam->m_fov, 60.0f);
  EXPECT_FLOAT_EQ(cam->m_near, 0.5f);
  EXPECT_FLOAT_EQ(cam->m_far, 500.0f);
  EXPECT_FLOAT_EQ(cam->m_width, 1920.0f);
  EXPECT_FLOAT_EQ(cam->m_height, 1080.0f);
  EXPECT_EQ(cam->m_position, glm::vec3(0.0f, 2.0f, 10.0f));
}

TEST_F(SceneLoaderTest, LoadsCameraWithDefaults)
{
  loader->init(kDefaultsFixture);

  auto cameraEntities = manager->view<CameraComponent>();
  ASSERT_EQ(cameraEntities.size(), 1u);
  auto* cam = manager->getComponent<CameraComponent>(cameraEntities[0]);
  ASSERT_NE(cam, nullptr);

  EXPECT_FLOAT_EQ(cam->m_fov, 45.0f);
  EXPECT_FLOAT_EQ(cam->m_near, 0.1f);
  EXPECT_FLOAT_EQ(cam->m_far, 200.0f);
  EXPECT_FLOAT_EQ(cam->m_width, 1600.0f);
  EXPECT_FLOAT_EQ(cam->m_height, 1200.0f);
  EXPECT_EQ(cam->m_position, glm::vec3(0.0f, 0.0f, 5.0f));
}

TEST_F(SceneLoaderTest, SaveSceneRoundTrip)
{
  Entity boxEntity = manager->createEntity("RoundTripBox");
  auto& pos = manager->emplaceComponent<PositionComponent>(boxEntity);
  pos.position = glm::vec3(4.0f, -1.0f, 2.5f);
  pos.scale = glm::vec3(1.0f, 1.0f, 1.0f);
  manager->emplaceComponent<PhysicsComponent>(
    boxEntity, boxEntity, 7.0f, CollisionShapeType::SPHERE);

  Entity lightEntity = manager->createEntity("RoundTripLight");
  manager->setupPointLight(lightEntity,
                           glm::vec3(1.0f, 0.5f, 0.25f),
                           1.0f,
                           0.09f,
                           0.032f,
                           glm::vec3(0.0f, 3.0f, 0.0f));

  const std::filesystem::path tempFile =
    std::filesystem::temp_directory_path() /
    "emengine_scene_loader_roundtrip_test.yaml";

  loader->saveScene(tempFile.string().c_str());

  manager->reset();
  ASSERT_EQ(manager->getEntities().size(), 0u);

  loader->init(tempFile.string().c_str());
  std::remove(tempFile.string().c_str());

  auto positionEntities = manager->view<PositionComponent>();
  ASSERT_EQ(positionEntities.size(), 1u);
  auto* reloadedPos = manager->getComponent<PositionComponent>(positionEntities[0]);
  ASSERT_NE(reloadedPos, nullptr);
  EXPECT_EQ(reloadedPos->position, glm::vec3(4.0f, -1.0f, 2.5f));

  auto physicsEntities = manager->view<PhysicsComponent>();
  ASSERT_EQ(physicsEntities.size(), 1u);
  auto* reloadedPhy = manager->getComponent<PhysicsComponent>(physicsEntities[0]);
  ASSERT_NE(reloadedPhy, nullptr);
  EXPECT_FLOAT_EQ(reloadedPhy->getMass(), 7.0f);
  EXPECT_EQ(reloadedPhy->getShapeType(), CollisionShapeType::SPHERE);

  auto lightEntities = manager->view<LightingComponent>();
  ASSERT_EQ(lightEntities.size(), 1u);
  auto* reloadedLight = manager->getComponent<LightingComponent>(lightEntities[0]);
  ASSERT_NE(reloadedLight, nullptr);
  EXPECT_EQ(reloadedLight->type, LightingComponent::TYPE::POINT);
  auto* reloadedPoint = static_cast<PointLight*>(reloadedLight->light.get());
  EXPECT_EQ(reloadedPoint->color, glm::vec3(1.0f, 0.5f, 0.25f));
}

TEST_F(SceneLoaderTest, SaveSceneThrowsOnUnsetLightingComponent)
{
  // Regression pin: saveScene() throws std::logic_error if any entity
  // carries a LightingComponent still at TYPE::NONE. Real foot-gun (skipping
  // the component would be saner than aborting the whole save), not fixed
  // here — pinned so a future fix is a deliberate test change, not a
  // silent regression.
  Entity entity = manager->createEntity("UnsetLight");
  auto baseLight = std::make_shared<BaseLight>();
  manager->emplaceComponent<LightingComponent>(
    entity, baseLight, LightingComponent::TYPE::NONE);

  const std::filesystem::path tempFile =
    std::filesystem::temp_directory_path() /
    "emengine_scene_loader_throw_test.yaml";

  EXPECT_THROW(loader->saveScene(tempFile.string().c_str()),
               std::logic_error);

  std::remove(tempFile.string().c_str());
}
