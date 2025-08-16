#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

// Comprehensive GLM Math Tests
class MathTest : public ::testing::Test
{};

// Vector3 Operations
TEST_F(MathTest, Vec3BasicOperations)
{
  glm::vec3 v1(1.0f, 2.0f, 3.0f);
  glm::vec3 v2(4.0f, 5.0f, 6.0f);

  // Addition
  glm::vec3 sum = v1 + v2;
  EXPECT_EQ(sum, glm::vec3(5.0f, 7.0f, 9.0f));

  // Subtraction
  glm::vec3 diff = v2 - v1;
  EXPECT_EQ(diff, glm::vec3(3.0f, 3.0f, 3.0f));

  // Scalar multiplication
  glm::vec3 scaled = v1 * 2.0f;
  EXPECT_EQ(scaled, glm::vec3(2.0f, 4.0f, 6.0f));

  // Component-wise multiplication
  glm::vec3 multiplied = v1 * v2;
  EXPECT_EQ(multiplied, glm::vec3(4.0f, 10.0f, 18.0f));
}

TEST_F(MathTest, Vec3DotProduct)
{
  glm::vec3 v1(1.0f, 2.0f, 3.0f);
  glm::vec3 v2(4.0f, 5.0f, 6.0f);

  float dot = glm::dot(v1, v2);
  EXPECT_FLOAT_EQ(dot, 32.0f); // 1*4 + 2*5 + 3*6 = 4 + 10 + 18 = 32
}

TEST_F(MathTest, Vec3CrossProduct)
{
  glm::vec3 v1(1.0f, 0.0f, 0.0f);
  glm::vec3 v2(0.0f, 1.0f, 0.0f);

  glm::vec3 cross = glm::cross(v1, v2);
  EXPECT_EQ(cross, glm::vec3(0.0f, 0.0f, 1.0f));
}

TEST_F(MathTest, Vec3Length)
{
  glm::vec3 v(3.0f, 4.0f, 0.0f);
  float length = glm::length(v);
  EXPECT_FLOAT_EQ(length, 5.0f); // 3-4-5 triangle

  // Test length squared for performance comparisons
  float lengthSq = glm::length2(v);
  EXPECT_FLOAT_EQ(lengthSq, 25.0f);
}

TEST_F(MathTest, Vec3Normalize)
{
  glm::vec3 v(3.0f, 4.0f, 0.0f);
  glm::vec3 normalized = glm::normalize(v);

  EXPECT_FLOAT_EQ(glm::length(normalized), 1.0f);
  EXPECT_FLOAT_EQ(normalized.x, 0.6f);
  EXPECT_FLOAT_EQ(normalized.y, 0.8f);
  EXPECT_FLOAT_EQ(normalized.z, 0.0f);
}

TEST_F(MathTest, Vec3Distance)
{
  glm::vec3 p1(1.0f, 2.0f, 3.0f);
  glm::vec3 p2(4.0f, 6.0f, 3.0f);

  float distance = glm::distance(p1, p2);
  EXPECT_FLOAT_EQ(distance,
                  5.0f); // sqrt((4-1)^2 + (6-2)^2 + (3-3)^2) = sqrt(9+16) = 5
}

TEST_F(MathTest, Vec3Lerp)
{
  glm::vec3 start(0.0f, 0.0f, 0.0f);
  glm::vec3 end(10.0f, 20.0f, 30.0f);

  glm::vec3 lerped = glm::mix(start, end, 0.5f);
  EXPECT_EQ(lerped, glm::vec3(5.0f, 10.0f, 15.0f));

  glm::vec3 lerpedQuarter = glm::mix(start, end, 0.25f);
  EXPECT_EQ(lerpedQuarter, glm::vec3(2.5f, 5.0f, 7.5f));
}

// Vector4 Operations
TEST_F(MathTest, Vec4Operations)
{
  glm::vec4 v1(1.0f, 2.0f, 3.0f, 4.0f);
  glm::vec4 v2(5.0f, 6.0f, 7.0f, 8.0f);

  glm::vec4 sum = v1 + v2;
  EXPECT_EQ(sum, glm::vec4(6.0f, 8.0f, 10.0f, 12.0f));

  float dot = glm::dot(v1, v2);
  EXPECT_FLOAT_EQ(dot, 70.0f); // 1*5 + 2*6 + 3*7 + 4*8 = 5 + 12 + 21 + 32 = 70
}

// Matrix4x4 Operations
TEST_F(MathTest, Mat4Identity)
{
  glm::mat4 identity = glm::mat4(1.0f);
  glm::vec4 testVec(1.0f, 2.0f, 3.0f, 1.0f);

  glm::vec4 result = identity * testVec;
  EXPECT_EQ(result, testVec);
}

TEST_F(MathTest, Mat4Translation)
{
  glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 10.0f, 15.0f));
  glm::vec4 point(1.0f, 2.0f, 3.0f, 1.0f);

  glm::vec4 translated = translation * point;
  EXPECT_EQ(translated, glm::vec4(6.0f, 12.0f, 18.0f, 1.0f));
}

TEST_F(MathTest, Mat4Scale)
{
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 3.0f, 4.0f));
  glm::vec4 point(1.0f, 1.0f, 1.0f, 1.0f);

  glm::vec4 scaled = scale * point;
  EXPECT_EQ(scaled, glm::vec4(2.0f, 3.0f, 4.0f, 1.0f));
}

TEST_F(MathTest, Mat4Rotation)
{
  glm::mat4 rotation = glm::rotate(
    glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
  glm::vec4 point(1.0f, 0.0f, 0.0f, 1.0f);

  glm::vec4 rotated = rotation * point;
  EXPECT_NEAR(rotated.x, 0.0f, 0.001f);
  EXPECT_NEAR(rotated.y, 1.0f, 0.001f);
  EXPECT_NEAR(rotated.z, 0.0f, 0.001f);
  EXPECT_NEAR(rotated.w, 1.0f, 0.001f);
}

TEST_F(MathTest, Mat4Multiplication)
{
  glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 0.0f, 0.0f));
  glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 2.0f, 2.0f));

  glm::mat4 combined = translation * scale;
  glm::vec4 point(1.0f, 1.0f, 0.0f, 1.0f);

  glm::vec4 transformed = combined * point;
  EXPECT_EQ(transformed,
            glm::vec4(7.0f, 2.0f, 0.0f, 1.0f)); // Scale first, then translate
}

TEST_F(MathTest, Mat4Inverse)
{
  glm::mat4 translation =
    glm::translate(glm::mat4(1.0f), glm::vec3(5.0f, 10.0f, 15.0f));
  glm::mat4 inverse = glm::inverse(translation);

  glm::mat4 identity = translation * inverse;

  // Check if result is approximately identity
  for (int i = 0; i < 4; ++i) {
    for (int j = 0; j < 4; ++j) {
      if (i == j) {
        EXPECT_NEAR(identity[i][j], 1.0f, 0.001f);
      } else {
        EXPECT_NEAR(identity[i][j], 0.0f, 0.001f);
      }
    }
  }
}

TEST_F(MathTest, Mat4Transpose)
{
  glm::mat4 mat(1.0f,
                2.0f,
                3.0f,
                4.0f,
                5.0f,
                6.0f,
                7.0f,
                8.0f,
                9.0f,
                10.0f,
                11.0f,
                12.0f,
                13.0f,
                14.0f,
                15.0f,
                16.0f);

  glm::mat4 transposed = glm::transpose(mat);

  // In GLM column-major layout, mat constructor creates:
  // Col0:(1,5,9,13), Col1:(2,6,10,14), Col2:(3,7,11,15), Col3:(4,8,12,16)
  // After transpose: Row0:(1,2,3,4), Row1:(5,6,7,8), Row2:(9,10,11,12),
  // Row3:(13,14,15,16)
  EXPECT_FLOAT_EQ(transposed[0][1],
                  5.0f); // Row 0, Col 1 = 2 -> Row 1, Col 0 = 5
  EXPECT_FLOAT_EQ(transposed[1][0],
                  2.0f); // Row 1, Col 0 = 6 -> Row 0, Col 1 = 2
  EXPECT_FLOAT_EQ(transposed[2][3], 15.0f); // Row 2, Col 3 = 15
  EXPECT_FLOAT_EQ(transposed[3][2], 12.0f); // Row 3, Col 2 = 12
}

// Quaternion Operations
TEST_F(MathTest, QuaternionIdentity)
{
  glm::quat identity = glm::identity<glm::quat>();

  EXPECT_NEAR(glm::length(identity), 1.0f, 0.001f);
  EXPECT_NEAR(identity.w, 1.0f, 0.001f);
  EXPECT_NEAR(identity.x, 0.0f, 0.001f);
  EXPECT_NEAR(identity.y, 0.0f, 0.001f);
  EXPECT_NEAR(identity.z, 0.0f, 0.001f);
}

TEST_F(MathTest, QuaternionFromAxisAngle)
{
  glm::quat rotation =
    glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  EXPECT_NEAR(glm::length(rotation), 1.0f, 0.001f);

  // For 90-degree rotation around Y-axis: w = cos(45°), y = sin(45°)
  EXPECT_NEAR(rotation.w, 0.7071f, 0.001f);
  EXPECT_NEAR(rotation.x, 0.0f, 0.001f);
  EXPECT_NEAR(rotation.y, 0.7071f, 0.001f);
  EXPECT_NEAR(rotation.z, 0.0f, 0.001f);
}

TEST_F(MathTest, QuaternionMultiplication)
{
  glm::quat rot1 =
    glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat rot2 =
    glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  glm::quat combined = rot1 * rot2;

  EXPECT_NEAR(glm::length(combined), 1.0f, 0.001f);

  // Two 45-degree rotations should equal one 90-degree rotation
  glm::quat expected =
    glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  EXPECT_NEAR(combined.w, expected.w, 0.001f);
  EXPECT_NEAR(combined.x, expected.x, 0.001f);
  EXPECT_NEAR(combined.y, expected.y, 0.001f);
  EXPECT_NEAR(combined.z, expected.z, 0.001f);
}

TEST_F(MathTest, QuaternionToMatrix)
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

TEST_F(MathTest, QuaternionSlerp)
{
  glm::quat start = glm::identity<glm::quat>();
  glm::quat end =
    glm::angleAxis(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  glm::quat halfway = glm::slerp(start, end, 0.5f);

  EXPECT_NEAR(glm::length(halfway), 1.0f, 0.001f);

  // Should be approximately 45-degree rotation
  glm::quat expected =
    glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));

  EXPECT_NEAR(halfway.w, expected.w, 0.01f);
  EXPECT_NEAR(halfway.y, expected.y, 0.01f);
}

TEST_F(MathTest, QuaternionConjugate)
{
  glm::quat rotation =
    glm::angleAxis(glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  glm::quat conjugate = glm::conjugate(rotation);

  // Conjugate should have same w, negated x,y,z
  EXPECT_NEAR(conjugate.w, rotation.w, 0.001f);
  EXPECT_NEAR(conjugate.x, -rotation.x, 0.001f);
  EXPECT_NEAR(conjugate.y, -rotation.y, 0.001f);
  EXPECT_NEAR(conjugate.z, -rotation.z, 0.001f);

  // rotation * conjugate should be identity
  glm::quat identity = rotation * conjugate;
  EXPECT_NEAR(identity.w, 1.0f, 0.001f);
  EXPECT_NEAR(identity.x, 0.0f, 0.001f);
  EXPECT_NEAR(identity.y, 0.0f, 0.001f);
  EXPECT_NEAR(identity.z, 0.0f, 0.001f);
}

// Projection and View Matrix Tests
TEST_F(MathTest, PerspectiveProjection)
{
  glm::mat4 projection =
    glm::perspective(glm::radians(45.0f), 16.0f / 9.0f, 0.1f, 100.0f);

  // Should not be identity
  EXPECT_NE(projection, glm::mat4(1.0f));

  // Test point transformation
  glm::vec4 point(0.0f, 0.0f, -1.0f, 1.0f); // Point 1 unit in front of camera
  glm::vec4 projected = projection * point;

  // After perspective division, should be in NDC space
  projected /= projected.w;
  EXPECT_TRUE(projected.z > -1.0f && projected.z < 1.0f);
}

TEST_F(MathTest, LookAtMatrix)
{
  glm::vec3 eye(0.0f, 0.0f, 5.0f);
  glm::vec3 center(0.0f, 0.0f, 0.0f);
  glm::vec3 up(0.0f, 1.0f, 0.0f);

  glm::mat4 view = glm::lookAt(eye, center, up);

  // Should not be identity
  EXPECT_NE(view, glm::mat4(1.0f));

  // Transform the eye position should give origin
  glm::vec4 transformedEye = view * glm::vec4(eye, 1.0f);
  EXPECT_NEAR(transformedEye.x, 0.0f, 0.001f);
  EXPECT_NEAR(transformedEye.y, 0.0f, 0.001f);
  EXPECT_NEAR(transformedEye.z, 0.0f, 0.001f);
}

// Utility and Edge Case Tests
TEST_F(MathTest, MathConstants)
{
  EXPECT_NEAR(glm::pi<float>(), 3.14159f, 0.001f);
  EXPECT_NEAR(glm::radians(180.0f), glm::pi<float>(), 0.001f);
  EXPECT_NEAR(glm::degrees(glm::pi<float>()), 180.0f, 0.001f);
}

TEST_F(MathTest, ClampFunction)
{
  EXPECT_FLOAT_EQ(glm::clamp(0.5f, 0.0f, 1.0f), 0.5f);
  EXPECT_FLOAT_EQ(glm::clamp(-0.5f, 0.0f, 1.0f), 0.0f);
  EXPECT_FLOAT_EQ(glm::clamp(1.5f, 0.0f, 1.0f), 1.0f);
}

TEST_F(MathTest, SmoothstepFunction)
{
  EXPECT_FLOAT_EQ(glm::smoothstep(0.0f, 1.0f, 0.0f), 0.0f);
  EXPECT_FLOAT_EQ(glm::smoothstep(0.0f, 1.0f, 1.0f), 1.0f);
  EXPECT_FLOAT_EQ(glm::smoothstep(0.0f, 1.0f, 0.5f), 0.5f);
}