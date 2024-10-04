#include "AnimationSystem.hpp"
#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/Components/GraphicsComponent.hpp>
#include <ECS/ECSManager.hpp>
#include <iostream>

void AnimationSystem::update(float dt) {
  // Find entities with Animation, Graphics components
  std::vector<Entity> view =
      m_manager->view<AnimationComponent, GraphicsComponent>();

  for (auto entity : view) {
    auto animComp = m_manager->getComponent<AnimationComponent>(entity);
    if (animComp->isPlaying) {
      auto graComp = m_manager->getComponent<GraphicsComponent>(entity);

      // Check if the graphics object has animations
      if (graComp->m_grapObj->p_numAnimations == 0) {
        std::cout << "Entity " << entity << " does not contain animation."
                  << std::endl;
        continue;
      }

      // Get the current animation
      Animation &animation =
          graComp->m_grapObj->p_animations[animComp->animationIndex];

      // Update the animation's current time
      animComp->currentTime += dt;
      if (animComp->currentTime > animation.end) {
        animComp->currentTime -= animation.end; // Loop animation
      }

      // Process each channel (e.g., translation, rotation, scale)
      for (auto &channel : animation.channels) {
        AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
        if (sampler.inputs.size() > sampler.outputsVec4.size()) {
          continue; // Invalid sampler, skip
        }

        // Interpolate between keyframes
        glm::vec3 translation{0.0f};
        glm::vec3 scale{1.0f};
        glm::quat rotation{1.0f, 0.0f, 0.0f, 0.0f};
        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
          if ((animComp->currentTime >= sampler.inputs[i]) &&
              (animComp->currentTime <= sampler.inputs[i + 1])) {
            float u =
                std::max(0.0f, animComp->currentTime - sampler.inputs[i]) /
                (sampler.inputs[i + 1] - sampler.inputs[i]);

            if (u <= 1.0f) {
              // Now apply the transformation to the node instead of the
              // PositionComponent
              switch (channel.path) {
              case AnimationChannel::PathType::TRANSLATION: {
                translation = sampler.translate(i, animComp->currentTime);
                graComp->m_grapObj->p_nodes[channel.node]->trans = translation;
                break;
              }
              case AnimationChannel::PathType::SCALE: {
                scale = sampler.scale(i, animComp->currentTime);
                graComp->m_grapObj->p_nodes[channel.node]->scale = scale;
                break;
              }
              case AnimationChannel::PathType::ROTATION: {
                rotation = sampler.rotate(i, animComp->currentTime);
                graComp->m_grapObj->p_nodes[channel.node]->rot = rotation;
                break;
              }
              }
            }
          }
        }
      }
    }
  }
}
