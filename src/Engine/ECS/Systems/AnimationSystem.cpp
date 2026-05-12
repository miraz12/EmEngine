#include "AnimationSystem.hpp"
#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <Objects/GraphicsObject.hpp>
#include <iostream>

void
AnimationSystem::sampleAnimation(GraphicsObject* obj,
                                 Animation& animation,
                                 float time)
{
  for (auto& channel : animation.channels) {
    AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
    if (sampler.keyframes.size() <= 1) {
      continue;
    }

    size_t keyframeIndex = sampler.findKeyframeIndex(time);

    switch (channel.path) {
      case AnimationChannel::PathType::TRANSLATION:
        obj->p_nodes[channel.node].trans =
          sampler.translate(keyframeIndex, time);
        break;
      case AnimationChannel::PathType::SCALE:
        obj->p_nodes[channel.node].scale = sampler.scale(keyframeIndex, time);
        break;
      case AnimationChannel::PathType::ROTATION:
        obj->p_nodes[channel.node].rot = sampler.rotate(keyframeIndex, time);
        break;
    }
  }
}

void
AnimationSystem::snapshotPose(GraphicsObject* obj)
{
  u32 n = obj->p_numNodes;
  if (m_snapTrans.size() < n) {
    m_snapTrans.resize(n);
    m_snapRot.resize(n);
    m_snapScale.resize(n);
  }
  for (u32 i = 0; i < n; i++) {
    m_snapTrans[i] = obj->p_nodes[i].trans;
    m_snapRot[i] = obj->p_nodes[i].rot;
    m_snapScale[i] = obj->p_nodes[i].scale;
  }
}

void
AnimationSystem::blendPose(GraphicsObject* obj, float weight)
{
  for (u32 i = 0; i < obj->p_numNodes; i++) {
    obj->p_nodes[i].trans =
      glm::mix(m_snapTrans[i], obj->p_nodes[i].trans, weight);
    obj->p_nodes[i].rot = glm::slerp(m_snapRot[i], obj->p_nodes[i].rot, weight);
    obj->p_nodes[i].scale =
      glm::mix(m_snapScale[i], obj->p_nodes[i].scale, weight);
  }
}

void
AnimationSystem::update(float dt)
{
  if (!m_manager->getRenderGraphics()) {
    return;
  }

  std::vector<Entity> view =
    m_manager->view<AnimationComponent, GraphicsComponent>();

  for (auto entity : view) {
    auto animComp = m_manager->getComponent<AnimationComponent>(entity);
    if (!animComp || !animComp->isPlaying) {
      continue;
    }

    auto graComp = m_manager->getComponent<GraphicsComponent>(entity);
    if (!graComp || !graComp->m_grapObj) {
      continue;
    }

    if (graComp->m_grapObj->p_numAnimations == 0) {
      if (!animComp->loggedNoAnimation) {
        std::cout << "Entity " << entity << " does not contain animation."
                  << '\n';
        animComp->loggedNoAnimation = true;
      }
      continue;
    }

    auto* obj = graComp->m_grapObj.get();

    if (animComp->blending) {
      // Advance blend timer
      animComp->blendElapsed += dt;
      animComp->blendWeight = std::clamp(
        animComp->blendElapsed / animComp->blendDuration, 0.0f, 1.0f);

      // Advance both animation clocks
      Animation& fromAnim = obj->p_animations[animComp->blendFromIndex];
      Animation& toAnim = obj->p_animations[animComp->animationIndex];

      animComp->blendFromTime += dt;
      if (animComp->blendFromTime > fromAnim.end) {
        animComp->blendFromTime -= fromAnim.end;
      }

      animComp->currentTime += dt;
      if (animComp->currentTime > toAnim.end) {
        animComp->currentTime -= toAnim.end;
      }

      // Sample source pose, snapshot it, then sample target pose
      sampleAnimation(obj, fromAnim, animComp->blendFromTime);
      snapshotPose(obj);
      sampleAnimation(obj, toAnim, animComp->currentTime);

      // Blend between source (snapshot) and target (current node TRS)
      blendPose(obj, animComp->blendWeight);

      // Complete the blend when duration elapsed
      if (animComp->blendElapsed >= animComp->blendDuration) {
        animComp->blending = false;
      }
    } else {
      // Single animation path
      Animation& animation = obj->p_animations[animComp->animationIndex];

      animComp->currentTime += dt;
      if (animComp->currentTime > animation.end) {
        animComp->currentTime -= animation.end;
      }

      sampleAnimation(obj, animation, animComp->currentTime);
    }

    obj->resetMatrixCache();
  }
}
