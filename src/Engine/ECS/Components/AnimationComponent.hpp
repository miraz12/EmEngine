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
  bool loggedNoAnimation{ false }; // Flag to avoid repeated logging
};

#endif // ANMATIONCOMPONENT_H_
