#ifndef LIGHTINGCOMPONENT_H_
#define LIGHTINGCOMPONENT_H_

#include "Component.hpp"
#include "Types/LightTypes.hpp"

class LightingComponent : public Component
{
public:
  enum class TYPE
  {
    NONE,
    POINT,
    DIRECTIONAL,
  };

  LightingComponent(const LightingComponent&) = default;
  LightingComponent(LightingComponent&&) = delete;
  LightingComponent& operator=(const LightingComponent&) = default;
  LightingComponent& operator=(LightingComponent&&) = delete;
  LightingComponent(std::shared_ptr<BaseLight> light, TYPE type)
    : m_t(type)
    , light(light)
  {
  }
  virtual ~LightingComponent() = default;

  TYPE getType() { return m_t; };
  BaseLight& getBaseLight() { return *light.get(); };

private:
  TYPE m_t{ TYPE::NONE };
  std::shared_ptr<BaseLight> light;
};
#endif // LIGHTINGCOMPONENT_H_
