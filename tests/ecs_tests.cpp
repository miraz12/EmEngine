#include "ECS/ECSManager.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Mock classes for systems
class MockSystem : public System {
public:
  MOCK_METHOD(void, initialize, (ECSManager & manager), (override));
  MOCK_METHOD(void, update, (float dt), (override));

private:
  MockSystem();
  ~MockSystem();
};

class MockGraphicsSystem : public GraphicsSystem {
public:
  MOCK_METHOD(void, setViewport, (u32 w, u32 h), (override));

private:
  MockGraphicsSystem();
  ~MockGraphicsSystem();
};

// Test ECSManager initialization
TEST(ECSManagerTest, InitializeSystems) {
  ECSManager &manager = ECSManager::getInstance();
}

// Test entity creation
TEST(ECSManagerTest, CreateEntity) {
  ECSManager &manager = ECSManager::getInstance();
  Entity entity = manager.createEntity("TestEntity");

  EXPECT_EQ(manager.getEntityName(entity), "TestEntity");
  // EXPECT_TRUE(manager.hasEntity(entity));
}

// // Test point light setup
TEST(ECSManagerTest, SetupPointLight) {
  ECSManager &manager = ECSManager::getInstance();
  Entity entity = manager.createEntity("LightEntity");

  glm::vec3 color(1.0f, 1.0f, 1.0f);
  float constant = 1.0f;
  float linear = 0.09f;
  float quadratic = 0.032f;
  glm::vec3 position(0.0f, 1.0f, 0.0f);

  std::shared_ptr<PointLight> pLight = manager.SetupPointLight(
      entity, color, constant, linear, quadratic, position);

  EXPECT_EQ(pLight->color, color);
  EXPECT_EQ(pLight->constant, constant);
  EXPECT_EQ(pLight->linear, linear);
  EXPECT_EQ(pLight->quadratic, quadratic);
  EXPECT_EQ(pLight->position, position);
}

// // Test directional light setup and update
TEST(ECSManagerTest, SetupAndUpdateDirectionalLight) {
  ECSManager &manager = ECSManager::getInstance();
  Entity entity = manager.createEntity("DirectionalLightEntity");

  glm::vec3 color(1.0f, 1.0f, 1.0f);
  float ambient = 0.5f;
  glm::vec3 direction(1.0f, -1.0f, 0.0f);

  std::shared_ptr<DirectionalLight> dLight =
      manager.SetupDirectionalLight(entity, color, ambient, direction);

  EXPECT_EQ(dLight->color, color);
  EXPECT_EQ(dLight->ambientIntensity, ambient);
  EXPECT_EQ(dLight->direction, direction);

  // Update directional light
  glm::vec3 newColor(0.5f, 0.5f, 0.5f);
  float newAmbient = 0.3f;
  glm::vec3 newDirection(0.0f, -1.0f, 1.0f);

  manager.updateDirLight(newColor, newAmbient, newDirection);

  EXPECT_EQ(dLight->color, newColor);
  EXPECT_EQ(dLight->ambientIntensity, newAmbient);
  EXPECT_EQ(dLight->direction, newDirection);
}
