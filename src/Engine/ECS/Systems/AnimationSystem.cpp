#include "AnimationSystem.hpp"
#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <iostream>

void
AnimationSystem::update(float dt)
{
  // Find entities with Animation, Graphics components
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

    // Check if the graphics object has animations
    if (graComp->m_grapObj->p_numAnimations == 0) {
      // Only log this once per entity
      if (!animComp->loggedNoAnimation) {
        std::cout << "Entity " << entity << " does not contain animation."
                  << std::endl;
        animComp->loggedNoAnimation = true;
      }
      continue;
    }

    // Get the current animation
    Animation& animation =
      graComp->m_grapObj->p_animations[animComp->animationIndex];

    // Update the animation's current time
    animComp->currentTime += dt;
    if (animComp->currentTime > animation.end) {
      animComp->currentTime -= animation.end; // Loop animation
    }

    // Process each channel (e.g., translation, rotation, scale)
    for (auto& channel : animation.channels) {
      AnimationSampler& sampler = animation.samplers[channel.samplerIndex];
      if (sampler.inputs.size() <= 1 ||
          sampler.inputs.size() > sampler.outputsVec4.size()) {
        continue; // Invalid sampler, skip
      }

      // Find the keyframe index for the current time
      size_t keyframeIndex = sampler.findKeyframeIndex(animComp->currentTime);

      // Apply the transformation to the node
      switch (channel.path) {
        case AnimationChannel::PathType::TRANSLATION: {
          glm::vec3 translation =
            sampler.translate(keyframeIndex, animComp->currentTime);
          graComp->m_grapObj->p_nodes[channel.node].trans = translation;
          break;
        }
        case AnimationChannel::PathType::SCALE: {
          glm::vec3 scale = sampler.scale(keyframeIndex, animComp->currentTime);
          graComp->m_grapObj->p_nodes[channel.node].scale = scale;
          break;
        }
        case AnimationChannel::PathType::ROTATION: {
          glm::quat rotation =
            sampler.rotate(keyframeIndex, animComp->currentTime);
          graComp->m_grapObj->p_nodes[channel.node].rot = rotation;
          break;
        }
      }
    }

    // Reset the matrix cache since we've modified node transforms
    graComp->m_grapObj->resetMatrixCache();
  }
}
