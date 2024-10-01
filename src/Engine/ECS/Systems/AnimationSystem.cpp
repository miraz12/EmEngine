#include "AnimationSystem.hpp"

#include "AnimationSystem.hpp"
#include <ECS/Components/AnimationComponent.hpp>
#include <ECS/Components/PositionComponent.hpp>
#include <ECS/ECSManager.hpp>

void AnimationSystem::update(float dt) {
  std::vector<Entity> view =
      m_manager
          ->view<AnimationComponent, PositionComponent, GraphicsComponent>();

  for (auto entity : view) {
    auto animComp = m_manager->getComponent<AnimationComponent>(entity);
    if (animComp->isPlaying) {
      auto graComp = m_manager->getComponent<GraphicsComponent>(entity);
      auto posComp = m_manager->getComponent<PositionComponent>(entity);

      if (graComp->m_grapObj->p_numAnimations == 0) {
        std::cout << "Entity " << entity << " does not contain animation."
                  << std::endl;
        return;
      }

      Animation &animation =
          graComp->m_grapObj->p_animations[animComp->animationIndex];

      animComp->currentTime += dt;
      if (animComp->currentTime > animation.end) {
        animComp->currentTime -= animation.end;
      }
      for (auto &channel : animation.channels) {
        AnimationSampler &sampler = animation.samplers[channel.samplerIndex];
        if (sampler.inputs.size() > sampler.outputsVec4.size()) {
          continue;
        }
        for (size_t i = 0; i < sampler.inputs.size() - 1; i++) {
          if ((animComp->currentTime >= sampler.inputs[i]) &&
              (animComp->currentTime <= sampler.inputs[i + 1])) {
            float u =
                std::max(0.0f, animComp->currentTime - sampler.inputs[i]) /
                (sampler.inputs[i + 1] - sampler.inputs[i]);
            if (u <= 1.0f) {
              switch (channel.path) {
              case AnimationChannel::PathType::TRANSLATION:
                posComp->position = sampler.translate(i, animComp->currentTime);
                break;
              case AnimationChannel::PathType::SCALE:
                posComp->scale = sampler.scale(i, animComp->currentTime);
                break;
              case AnimationChannel::PathType::ROTATION:
                posComp->rotation = sampler.rotate(i, animComp->currentTime);
                break;
              }
            }
          }
        }
      }
    }
  }
}
