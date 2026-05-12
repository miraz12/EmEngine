#ifndef ANMATIONCOMPONENT_H_
#define ANMATIONCOMPONENT_H_
#include "Component.hpp"
#include <Rendering/Animation.hpp>

class AnimationComponent : public Component
{
public:
  AnimationComponent() = default;

  float currentTime{ 0.0f };
  bool isPlaying{ true };
  u32 animationIndex{ 0 };

  // Blend / crossfade state
  bool blending{ false };
  u32 blendFromIndex{ 0 };
  float blendFromTime{ 0.0f };
  float blendWeight{ 0.0f };
  float blendDuration{ 0.0f };
  float blendElapsed{ 0.0f };

  bool loggedNoAnimation{ false }; // Flag to avoid repeated logging
};

#endif // ANMATIONCOMPONENT_H_
