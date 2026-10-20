#include "SkinningGather.hpp"

#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Graphics/RenderResources.hpp>
#include <Objects/GraphicsObject.hpp>
#include <Rendering/SkeletonPose.hpp>
#include <unordered_map>

namespace {

std::vector<SkinnedNodeOffset> g_frameSkinning;
bool g_gathered = false;

// Bind-pose fallback for a skinned entity that has no AnimationComponent
// (a rigged model that is never animated). Keyed by GraphicsObject so all
// such entities of one model share it — they are all in bind pose, so
// unlike animated entities they cannot diverge.
std::unordered_map<GraphicsObject*, SkeletonPose>&
staticPoses()
{
  static std::unordered_map<GraphicsObject*, SkeletonPose> poses;
  return poses;
}

} // namespace

const std::vector<SkinnedNodeOffset>&
gatherAndUploadFrameSkinning(ECSManager& eManager)
{
  if (g_gathered) {
    return g_frameSkinning;
  }

  auto& resources = gfx::RenderResources::getInstance();
  resources.beginJointUploadFrame();
  g_frameSkinning.clear();

  eManager.each<GraphicsComponent>(
    [&](Entity entity, GraphicsComponent& gfxComp) {
      auto* obj = gfxComp.m_grapObj.get();
      if (!obj) {
        return;
      }

      SkeletonPose* pose = nullptr;
      if (auto* animComp = eManager.getComponent<AnimationComponent>(entity)) {
        animComp->pose.bindTo(*obj);
        pose = &animComp->pose;
      }

      for (u32 nodeIdx = 0; nodeIdx < obj->p_numNodes; nodeIdx++) {
        const i32 skin = obj->p_nodes[nodeIdx].skin;
        if (skin < 0) {
          continue;
        }

        if (!pose) {
          SkeletonPose& fallback = staticPoses()[obj];
          fallback.bindTo(*obj);
          pose = &fallback;
        }

        const auto& matrices = pose->jointMatrices(*obj, skin);
        if (matrices.empty()) {
          continue;
        }

        u32 offset = resources.appendJointMatrices(
          matrices.data(), static_cast<u32>(matrices.size()));
        g_frameSkinning.push_back({ entity, nodeIdx, offset });
      }
    });

  resources.flushJointUploads();
  g_gathered = true;
  return g_frameSkinning;
}

u32
findJointOffset(const std::vector<SkinnedNodeOffset>& frameSkinning,
                Entity entity,
                u32 nodeIdx)
{
  for (const auto& entry : frameSkinning) {
    if (entry.entity == entity && entry.nodeIdx == nodeIdx) {
      return entry.jointOffset;
    }
  }
  return 0;
}

void
resetFrameSkinningGather()
{
  g_gathered = false;
}
