#ifndef LIGHTINGCOMPONENT_H_
#define LIGHTINGCOMPONENT_H_

#include "Types/LightTypes.hpp"

struct LightingComponent
{
  enum class TYPE
  {
    NONE,
    POINT,
    DIRECTIONAL,
  };

  LightingComponent(const LightingComponent&) = default;
  LightingComponent(LightingComponent&&) = default;
  LightingComponent& operator=(const LightingComponent&) = default;
  LightingComponent& operator=(LightingComponent&&) = default;
  LightingComponent(std::shared_ptr<BaseLight> light, TYPE type)
    : type(type)
    , light(std::move(light))
  {
  }
  ~LightingComponent() = default;

  TYPE type{ TYPE::NONE };
  std::shared_ptr<BaseLight> light;
};
#endif // LIGHTINGCOMPONENT_H_
