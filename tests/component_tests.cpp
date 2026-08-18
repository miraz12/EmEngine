#include <glm/glm.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "ECS/Components/AnimationComponent.hpp"
#include "ECS/Components/LightingComponent.hpp"
#include "ECS/Components/PositionComponent.hpp"
#include "Types/LightTypes.hpp"

// Stub implementation for Game_Update that the engine expects
extern "C" void
Game_Update(float dt)
{
  (void)dt;
}

// PositionComponent Tests
class PositionComponentTest : public ::testing::Test
{
protected:
  void SetUp() override { component = PositionComponent{}; }
  PositionComponent component;
};

TEST_F(PositionComponentTest, DefaultInitialization)
{
  EXPECT_EQ(component.position, glm::vec3(0.0f));
  EXPECT_EQ(component.scale, glm::vec3(1.0f));
  // Test that rotation is identity quaternion
  EXPECT_NEAR(component.rotation.w, 1.0f, 0.001f);
  EXPECT_NEAR(component.rotation.x, 0.0f, 0.001f);
  EXPECT_NEAR(component.rotation.y, 0.0f, 0.001f);
  EXPECT_NEAR(component.rotation.z, 0.0f, 0.001f);
}

TEST_F(PositionComponentTest, SetPosition)
{
  glm::vec3 newPos(1.0f, 2.0f, 3.0f);
  component.position = newPos;
  EXPECT_EQ(component.position, newPos);
}

TEST_F(PositionComponentTest, SetRotation)
{
  glm::quat newRot(0.707f, 0.0f, 0.707f, 0.0f);
  component.rotation = newRot;
  EXPECT_EQ(component.rotation, newRot);
}

TEST_F(PositionComponentTest, SetScale)
{
  glm::vec3 newScale(2.0f, 2.0f, 2.0f);
  component.scale = newScale;
  EXPECT_EQ(component.scale, newScale);
}

TEST_F(PositionComponentTest, ConstructorWithParameters)
{
  PositionComponent comp2(5.0f, 10.0f);
  EXPECT_EQ(comp2.position, glm::vec3(5.0f, 10.0f, -0.1f));
}

// AnimationComponent Tests
class AnimationComponentTest : public ::testing::Test
{
protected:
  void SetUp() override { component = AnimationComponent{}; }
  AnimationComponent component;
};

TEST_F(AnimationComponentTest, DefaultInitialization)
{
  EXPECT_FLOAT_EQ(component.currentTime, 0.0f);
  EXPECT_TRUE(component.isPlaying);
  EXPECT_EQ(component.animationIndex, 0);
  EXPECT_FALSE(component.loggedNoAnimation);

  // Blend state defaults
  EXPECT_FALSE(component.blending);
  EXPECT_EQ(component.blendFromIndex, 0u);
  EXPECT_FLOAT_EQ(component.blendFromTime, 0.0f);
  EXPECT_FLOAT_EQ(component.blendWeight, 0.0f);
  EXPECT_FLOAT_EQ(component.blendDuration, 0.0f);
  EXPECT_FLOAT_EQ(component.blendElapsed, 0.0f);
}

TEST_F(AnimationComponentTest, FieldAssignment)
{
  component.animationIndex = 1;
  component.isPlaying = false;
  component.currentTime = 1.5f;
  component.loggedNoAnimation = true;

  EXPECT_EQ(component.animationIndex, 1);
  EXPECT_FALSE(component.isPlaying);
  EXPECT_FLOAT_EQ(component.currentTime, 1.5f);
  EXPECT_TRUE(component.loggedNoAnimation);
}

// LightingComponent Tests
class LightingComponentTest : public ::testing::Test
{};

TEST_F(LightingComponentTest, PointLightComponent)
{
  auto pointLight = std::make_shared<PointLight>();
  pointLight->color = glm::vec3(1.0f, 0.5f, 0.0f);
  pointLight->position = glm::vec3(0.0f, 2.0f, 0.0f);
  pointLight->constant = 1.0f;
  pointLight->linear = 0.09f;
  pointLight->quadratic = 0.032f;

  LightingComponent comp(pointLight, LightingComponent::TYPE::POINT);
  EXPECT_EQ(comp.type, LightingComponent::TYPE::POINT);
  EXPECT_EQ(comp.light->color, glm::vec3(1.0f, 0.5f, 0.0f));
}

TEST_F(LightingComponentTest, DirectionalLightComponent)
{
  auto dirLight = std::make_shared<DirectionalLight>();
  dirLight->color = glm::vec3(1.0f, 1.0f, 0.8f);
  dirLight->direction = glm::vec3(-0.5f, -1.0f, -0.3f);
  dirLight->intensity = 0.3f;

  LightingComponent comp(dirLight, LightingComponent::TYPE::DIRECTIONAL);
  EXPECT_EQ(comp.type, LightingComponent::TYPE::DIRECTIONAL);
  EXPECT_EQ(comp.light->color, glm::vec3(1.0f, 1.0f, 0.8f));
}

TEST_F(LightingComponentTest, BaseLightComponent)
{
  auto baseLight = std::make_shared<BaseLight>();
  baseLight->color = glm::vec3(0.8f, 0.8f, 1.0f);

  LightingComponent comp(baseLight, LightingComponent::TYPE::NONE);
  EXPECT_EQ(comp.type, LightingComponent::TYPE::NONE);
  EXPECT_EQ(comp.light->color, glm::vec3(0.8f, 0.8f, 1.0f));

  // Test modifying light through reference
  BaseLight& light = *comp.light;
  light.color = glm::vec3(1.0f, 0.0f, 0.0f);
  EXPECT_EQ(comp.light->color, glm::vec3(1.0f, 0.0f, 0.0f));
}

