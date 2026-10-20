#include <glm/glm.hpp>
#include <gtest/gtest.h>

#include "Objects/GraphicsObject.hpp"
#include "Rendering/SkeletonPose.hpp"

// Regression tests for animated model instancing.
//
// ResourceManager caches one GltfObject per filename, so every entity using
// the same animated model shares a single GraphicsObject. Before SkeletonPose
// existed, AnimationSystem wrote the sampled pose straight into that shared
// object, so N entities could never hold different poses — they all rendered
// whichever entity updated last. These tests pin the invariant that per-entity
// poses stay independent of each other and of the shared object.

namespace {

// Builds a minimal two-node rig: node 0 is the root/joint, node 1 is a
// mesh node skinned to it.
std::unique_ptr<GraphicsObject>
makeSkinnedObject()
{
  auto obj = std::make_unique<GraphicsObject>();

  obj->p_numNodes = 2;
  obj->p_nodes = std::make_unique<Node[]>(2);

  obj->p_nodes[0].mesh = -1;
  obj->p_nodes[0].skin = -1;
  obj->p_nodes[0].parent = -1;

  obj->p_nodes[1].mesh = 0;
  obj->p_nodes[1].skin = 0;
  obj->p_nodes[1].parent = -1;

  obj->p_numSkins = 1;
  obj->p_skins = std::make_unique<Skin[]>(1);
  obj->p_skins[0].joints = { 0 };
  obj->p_skins[0].inverseBindMatrices = { glm::mat4(1.0f) };

  return obj;
}

} // namespace

TEST(SkeletonPoseTest, SeedsFromObjectBindPose)
{
  auto obj = makeSkinnedObject();
  obj->p_nodes[0].trans = glm::vec3(1.0f, 2.0f, 3.0f);

  SkeletonPose pose;
  pose.bindTo(*obj);

  EXPECT_TRUE(pose.isBound());
  EXPECT_EQ(pose.nodeCount(), 2u);
  EXPECT_EQ(pose.translation(0), glm::vec3(1.0f, 2.0f, 3.0f));
}

TEST(SkeletonPoseTest, TwoPosesOnSharedObjectStayIndependent)
{
  auto obj = makeSkinnedObject();

  SkeletonPose poseA;
  SkeletonPose poseB;
  poseA.bindTo(*obj);
  poseB.bindTo(*obj);

  // Simulate two entities at different animation times.
  poseA.translation(0) = glm::vec3(10.0f, 0.0f, 0.0f);
  poseB.translation(0) = glm::vec3(-10.0f, 0.0f, 0.0f);
  poseA.invalidate();
  poseB.invalidate();

  glm::mat4 a = poseA.getMatrix(*obj, 0);
  glm::mat4 b = poseB.getMatrix(*obj, 0);

  EXPECT_EQ(a[3][0], 10.0f);
  EXPECT_EQ(b[3][0], -10.0f);
  EXPECT_NE(a, b);

  // The shared object must not have been mutated by either pose.
  EXPECT_EQ(obj->p_nodes[0].trans, glm::vec3(0.0f));
}

TEST(SkeletonPoseTest, JointMatricesReflectOwnPoseOnly)
{
  auto obj = makeSkinnedObject();

  SkeletonPose poseA;
  SkeletonPose poseB;
  poseA.bindTo(*obj);
  poseB.bindTo(*obj);

  poseA.translation(0) = glm::vec3(5.0f, 0.0f, 0.0f);
  poseB.translation(0) = glm::vec3(0.0f, 7.0f, 0.0f);
  poseA.invalidate();
  poseB.invalidate();

  const auto& jointsA = poseA.jointMatrices(*obj, 0);
  const auto& jointsB = poseB.jointMatrices(*obj, 0);

  ASSERT_EQ(jointsA.size(), 1u);
  ASSERT_EQ(jointsB.size(), 1u);

  // Each skeleton carries its own translation — this is exactly what used
  // to collapse to a single shared value.
  EXPECT_EQ(jointsA[0][3][0], 5.0f);
  EXPECT_EQ(jointsA[0][3][1], 0.0f);
  EXPECT_EQ(jointsB[0][3][0], 0.0f);
  EXPECT_EQ(jointsB[0][3][1], 7.0f);
}

TEST(SkeletonPoseTest, InvalidateRecomputesJointMatrices)
{
  auto obj = makeSkinnedObject();

  SkeletonPose pose;
  pose.bindTo(*obj);

  pose.translation(0) = glm::vec3(1.0f, 0.0f, 0.0f);
  pose.invalidate();
  EXPECT_EQ(pose.jointMatrices(*obj, 0)[0][3][0], 1.0f);

  // Without invalidate() the cached value must persist...
  pose.translation(0) = glm::vec3(2.0f, 0.0f, 0.0f);
  EXPECT_EQ(pose.jointMatrices(*obj, 0)[0][3][0], 1.0f);

  // ...and after invalidate() it must pick up the new pose.
  pose.invalidate();
  EXPECT_EQ(pose.jointMatrices(*obj, 0)[0][3][0], 2.0f);
}

TEST(SkeletonPoseTest, RespectsParentChain)
{
  auto obj = makeSkinnedObject();
  // Re-parent the mesh node under the root so the chain is exercised.
  obj->p_nodes[1].parent = 0;

  SkeletonPose pose;
  pose.bindTo(*obj);
  pose.translation(0) = glm::vec3(1.0f, 0.0f, 0.0f);
  pose.translation(1) = glm::vec3(0.0f, 2.0f, 0.0f);
  pose.invalidate();

  glm::mat4 child = pose.getMatrix(*obj, 1);
  EXPECT_EQ(child[3][0], 1.0f); // inherited from parent
  EXPECT_EQ(child[3][1], 2.0f); // own translation
}
