#include "AnimationSystem.hpp"
#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Objects/GraphicsObject.hpp>
#include <Rendering/SkeletonPose.hpp>
#include <iostream>

void
AnimationSystem::sampleAnimation(SkeletonPose& pose,
                                 Animation& animation,
                                 float time)
{
  for (auto& channel : animation.channels) {
    AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
    if (sampler.keyframes.size() <= 1) {
      continue;
    }

    size_t keyframeIndex =
      sampler.findKeyframeIndex(time, pose.keyframeHint(channel.samplerIndex));
    pose.setKeyframeHint(channel.samplerIndex, keyframeIndex);

    switch (channel.path) {
      case AnimationChannel::PathType::TRANSLATION:
        pose.translation(channel.node) = sampler.translate(keyframeIndex, time);
        break;
      case AnimationChannel::PathType::SCALE:
        pose.scale(channel.node) = sampler.scale(keyframeIndex, time);
        break;
      case AnimationChannel::PathType::ROTATION:
        pose.rotation(channel.node) = sampler.rotate(keyframeIndex, time);
        break;
    }
  }
}

void
AnimationSystem::snapshotPose(SkeletonPose& pose)
{
  u32 n = pose.nodeCount();
  if (m_snapTrans.size() < n) {
    m_snapTrans.resize(n);
    m_snapRot.resize(n);
    m_snapScale.resize(n);
  }
  for (u32 i = 0; i < n; i++) {
    m_snapTrans[i] = pose.translation(i);
    m_snapRot[i] = pose.rotation(i);
    m_snapScale[i] = pose.scale(i);
  }
}

void
AnimationSystem::blendPose(SkeletonPose& pose, float weight)
{
  for (u32 i = 0; i < pose.nodeCount(); i++) {
    pose.translation(i) = glm::mix(m_snapTrans[i], pose.translation(i), weight);
    pose.rotation(i) = glm::slerp(m_snapRot[i], pose.rotation(i), weight);
    pose.scale(i) = glm::mix(m_snapScale[i], pose.scale(i), weight);
  }
}

void
AnimationSystem::update(float dt)
{
  if (!m_manager->getRenderGraphics()) {
    return;
  }

  m_manager->each<AnimationComponent, GraphicsComponent>(
    [&](
      Entity entity, AnimationComponent& animComp, GraphicsComponent& graComp) {
      if (!animComp.isPlaying || !graComp.m_grapObj) {
        return;
      }

      if (graComp.m_grapObj->p_numAnimations == 0) {
        if (!animComp.loggedNoAnimation) {
          std::cout << "Entity " << entity << " does not contain animation."
                    << '\n';
          animComp.loggedNoAnimation = true;
        }
        return;
      }

      auto* obj = graComp.m_grapObj.get();

      // Bind this entity's pose to the (shared) model on first use.
      animComp.pose.bindTo(*obj);
      SkeletonPose& pose = animComp.pose;

      if (animComp.blending) {
        // Advance blend timer
        animComp.blendElapsed += dt;
        animComp.blendWeight = std::clamp(
          animComp.blendElapsed / animComp.blendDuration, 0.0f, 1.0f);

        // Advance both animation clocks
        Animation& fromAnim = obj->p_animations[animComp.blendFromIndex];
        Animation& toAnim = obj->p_animations[animComp.animationIndex];

        animComp.blendFromTime += dt;
        if (animComp.blendFromTime > fromAnim.end) {
          animComp.blendFromTime -= fromAnim.end;
        }

        animComp.currentTime += dt;
        if (animComp.currentTime > toAnim.end) {
          animComp.currentTime -= toAnim.end;
        }

        // Sample source pose, snapshot it, then sample target pose
        sampleAnimation(pose, fromAnim, animComp.blendFromTime);
        snapshotPose(pose);
        sampleAnimation(pose, toAnim, animComp.currentTime);

        // Blend between source (snapshot) and target (current node TRS)
        blendPose(pose, animComp.blendWeight);

        // Complete the blend when duration elapsed
        if (animComp.blendElapsed >= animComp.blendDuration) {
          animComp.blending = false;
        }
      } else {
        // Single animation path
        Animation& animation = obj->p_animations[animComp.animationIndex];

        animComp.currentTime += dt;
        if (animComp.currentTime > animation.end) {
          animComp.currentTime -= animation.end;
        }

        sampleAnimation(pose, animation, animComp.currentTime);
      }

      pose.invalidate();
    });
}
