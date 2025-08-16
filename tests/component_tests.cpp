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
  void SetUp() override { component = std::make_shared<PositionComponent>(); }
  std::shared_ptr<PositionComponent> component;
};

TEST_F(PositionComponentTest, DefaultInitialization)
{
  EXPECT_EQ(component->position, glm::vec3(0.0f));
  EXPECT_EQ(component->scale, glm::vec3(1.0f));
  // Test that rotation is identity quaternion
  EXPECT_NEAR(component->rotation.w, 1.0f, 0.001f);
  EXPECT_NEAR(component->rotation.x, 0.0f, 0.001f);
  EXPECT_NEAR(component->rotation.y, 0.0f, 0.001f);
  EXPECT_NEAR(component->rotation.z, 0.0f, 0.001f);
}

TEST_F(PositionComponentTest, SetPosition)
{
  glm::vec3 newPos(1.0f, 2.0f, 3.0f);
  component->position = newPos;
  EXPECT_EQ(component->position, newPos);
}

TEST_F(PositionComponentTest, SetRotation)
{
  glm::quat newRot(0.707f, 0.0f, 0.707f, 0.0f);
  component->rotation = newRot;
  EXPECT_EQ(component->rotation, newRot);
}

TEST_F(PositionComponentTest, SetScale)
{
  glm::vec3 newScale(2.0f, 2.0f, 2.0f);
  component->scale = newScale;
  EXPECT_EQ(component->scale, newScale);
}

TEST_F(PositionComponentTest, ConstructorWithParameters)
{
  auto component2 = std::make_shared<PositionComponent>(5.0f, 10.0f);
  EXPECT_EQ(component2->position, glm::vec3(5.0f, 10.0f, -0.1f));
}

// AnimationComponent Tests
class AnimationComponentTest : public ::testing::Test
{
protected:
  void SetUp() override { component = std::make_shared<AnimationComponent>(); }
  std::shared_ptr<AnimationComponent> component;
};

TEST_F(AnimationComponentTest, DefaultInitialization)
{
  EXPECT_FLOAT_EQ(component->currentTime, 0.0f);
  EXPECT_TRUE(component->isPlaying);
  EXPECT_EQ(component->animationIndex, 0);
  EXPECT_FALSE(component->loggedNoAnimation);
}

TEST_F(AnimationComponentTest, SetAnimationIndex)
{
  component->animationIndex = 1;
  EXPECT_EQ(component->animationIndex, 1);
}

TEST_F(AnimationComponentTest, SetPlayingState)
{
  component->isPlaying = false;
  EXPECT_FALSE(component->isPlaying);
}

TEST_F(AnimationComponentTest, UpdateTime)
{
  float newTime = 1.5f;
  component->currentTime = newTime;
  EXPECT_FLOAT_EQ(component->currentTime, newTime);
}

TEST_F(AnimationComponentTest, SetLoggedFlag)
{
  component->loggedNoAnimation = true;
  EXPECT_TRUE(component->loggedNoAnimation);
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

  auto component = std::make_shared<LightingComponent>(
    pointLight, LightingComponent::TYPE::POINT);
  EXPECT_EQ(component->getType(), LightingComponent::TYPE::POINT);
  EXPECT_EQ(component->getBaseLight().color, glm::vec3(1.0f, 0.5f, 0.0f));
}

TEST_F(LightingComponentTest, DirectionalLightComponent)
{
  auto dirLight = std::make_shared<DirectionalLight>();
  dirLight->color = glm::vec3(1.0f, 1.0f, 0.8f);
  dirLight->direction = glm::vec3(-0.5f, -1.0f, -0.3f);
  dirLight->ambientIntensity = 0.3f;

  auto component = std::make_shared<LightingComponent>(
    dirLight, LightingComponent::TYPE::DIRECTIONAL);
  EXPECT_EQ(component->getType(), LightingComponent::TYPE::DIRECTIONAL);
  EXPECT_EQ(component->getBaseLight().color, glm::vec3(1.0f, 1.0f, 0.8f));
}

TEST_F(LightingComponentTest, BaseLightComponent)
{
  auto baseLight = std::make_shared<BaseLight>();
  baseLight->color = glm::vec3(0.8f, 0.8f, 1.0f);

  auto component = std::make_shared<LightingComponent>(
    baseLight, LightingComponent::TYPE::NONE);
  EXPECT_EQ(component->getType(), LightingComponent::TYPE::NONE);
  EXPECT_EQ(component->getBaseLight().color, glm::vec3(0.8f, 0.8f, 1.0f));

  // Test modifying light through reference
  BaseLight& light = component->getBaseLight();
  light.color = glm::vec3(1.0f, 0.0f, 0.0f);
  EXPECT_EQ(component->getBaseLight().color, glm::vec3(1.0f, 0.0f, 0.0f));
}

// GLM Math Integration Tests
class GLMIntegrationTest : public ::testing::Test
{};

TEST_F(GLMIntegrationTest, Vec3Operations)
{
  glm::vec3 v1(1.0f, 2.0f, 3.0f);
  glm::vec3 v2(4.0f, 5.0f, 6.0f);

  glm::vec3 sum = v1 + v2;
  EXPECT_EQ(sum, glm::vec3(5.0f, 7.0f, 9.0f));

  glm::vec3 diff = v2 - v1;
  EXPECT_EQ(diff, glm::vec3(3.0f, 3.0f, 3.0f));

  float dot = glm::dot(v1, v2);
  EXPECT_FLOAT_EQ(dot, 32.0f); // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
}

TEST_F(GLMIntegrationTest, Vec3Length)
{
  glm::vec3 v(3.0f, 4.0f, 0.0f);
  float length = glm::length(v);
  EXPECT_FLOAT_EQ(length, 5.0f); // 3-4-5 triangle
}

TEST_F(GLMIntegrationTest, Vec3Normalize)
{
  glm::vec3 v(3.0f, 4.0f, 0.0f);
  glm::vec3 normalized = glm::normalize(v);
  EXPECT_FLOAT_EQ(glm::length(normalized), 1.0f);
  EXPECT_FLOAT_EQ(normalized.x, 0.6f);
  EXPECT_FLOAT_EQ(normalized.y, 0.8f);
}

TEST_F(GLMIntegrationTest, Mat4Operations)
{
  glm::mat4 identity = glm::mat4(1.0f);
  glm::vec4 testVec(1.0f, 2.0f, 3.0f, 1.0f);

  glm::vec4 result = identity * testVec;
  EXPECT_EQ(result, testVec);

  glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  glm::vec4 translated = translation * testVec;
  EXPECT_EQ(translated, glm::vec4(2.0f, 2.0f, 3.0f, 1.0f));
}

TEST_F(GLMIntegrationTest, Mat4Scale)
{
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 4.0f));
  glm::vec4 testVec(1.0f, 1.0f, 1.0f, 1.0f);

  glm::vec4 scaled = scale * testVec;
  EXPECT_EQ(scaled, glm::vec4(2.0f, 3.0f, 4.0f, 1.0f));
}

TEST_F(GLMIntegrationTest, QuaternionOperations)
{
  glm::quat identity = glm::identity<glm::quat>();
  glm::quat rotation =
    glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  // Test that quaternions are normalized
  EXPECT_NEAR(glm::length(identity), 1.0f, 0.001f);
  EXPECT_NEAR(glm::length(rotation), 1.0f, 0.001f);

  // Test quaternion multiplication (composition)
  glm::quat composed = rotation * identity;
  EXPECT_NEAR(glm::length(composed), 1.0f, 0.001f);
}

TEST_F(GLMIntegrationTest, QuaternionToMatrix)
{
  glm::quat rotation =
    glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::mat4 rotMatrix = glm::mat4_cast(rotation);

  // Apply 90-degree Y rotation to point (1,0,0) should give approximately
  // (0,0,-1)
  glm::vec4 point(1.0f, 0.0f, 0.0f, 1.0f);
  glm::vec4 rotated = rotMatrix * point;

  EXPECT_NEAR(rotated.x, 0.0f, 0.001f);
  EXPECT_NEAR(rotated.y, 0.0f, 0.001f);
  EXPECT_NEAR(rotated.z, -1.0f, 0.001f);
  EXPECT_NEAR(rotated.w, 1.0f, 0.001f);
}